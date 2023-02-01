#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/shm.h>
#include <sys/mman.h>

#include "../include/includeF.h"
#include "../include/includeL.h"
#include "../include/includeS.h"


/* Connects to the REGISTRO PIPE and reads the map data from it.
   The map data is stored in the `dest` array.
   Each element in the `dest` array is a string containing the path of a train.
   The connection to the REGISTRO PIPE is closed after the map data is read. */
void rbcMaps(char *dest[N_TRAINS]) {
  int registroPipe = connectToFifo(PIPE_FORMAT, N_RBC_PIPE);
  do {
    char buffer[512] = {0};
    if (read(registroPipe, buffer, sizeof(buffer)) == -1) {
      throwError("Error reading from REGISTRO PIPE");
    }
    char *map = strdup(buffer);
    // Copy itineraries from map into dest
    char *path;
    int i = 0;
    while ((path = strsep(&map, "~"))) {
      dest[i++] = strdup(path);
    }
    free(map);
    free(path);
  } while (false);
  close(registroPipe);
  printf("RBC Connection to registro pipe (fd=%d) interrupted.\n", registroPipe);
}

void rbcDataInit(rbcData_t *rbcData) {
    int stationNum;
    char *str_ptr, *stationName;
    // Set all segments to false
    for (int i = 0; i < N_SEGM; i++) {
        rbcData->segms[i] = false;
    }
    // Get map data and store it in rbcData->paths
    rbcMaps(rbcData->paths);
    // Set all stations to 0
    for (int i = 0; i < N_STATIONS; i++) {
        rbcData->stations[i] = 0;
    }
    // Iterate through all trains
    for (int i = 0; i < N_TRAINS; i++) {
        // Duplicate the train's path string
        str_ptr = strdup(rbcData->paths[i]);
        // Get the first station in the train's path
        stationName = strsep(&str_ptr, "-");
        // If the first station is a valid station, increment the count for that station
        if (stationVerifier(stationName)) {
            sscanf(stationName, "S%d", &stationNum);
            rbcData->stations[stationNum - 1]++;
        }
    }
    // Free the duplicated station name string
    free(stationName);
}


/* RBC server, socket creation
  Function to create a server socket for the RBC process.
  This function creates a socket using the AF_UNIX domain and the SOCK_STREAM type, and binds
  it to a local server address specified by the SERVER_NAME constant. It then starts listening
  for requests on the socket, and returns the file descriptor for the socket. */
int rbcServerSocket() {
    // Create socket
    int fd;
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        throwError("Failed to create socket");
    }
    struct sockaddr_un serverAddr;
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, SERVER_NAME);
    // Bind socket to server address
    if (bind(fd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == -1) {
        throwError("Failed to bind socket to server address");
    }
    // Start listening for requests on the socket
    if (listen(fd, N_TRAINS) == -1) {
        throwError("Failed to start listening on socket");
    }
    // Return the file descriptor for the socket
    return fd;
}

// Returns true if the segment has the correct status in the `rbcData` data structure, false otherwise.
bool segmStatusChecker(rbcData_t *rbcData, int segmentID, char *segmentName, bool station) {
    // If this is a station, return true
    if (station) return true; 
    // Get the value of the segment's status in the segment file
    const bool segmentFileValue = !isSegmentFree(segmentName);
    // Get the value of the segment's status in the RBC data
    const bool rbcSegmentFileValue = rbcData->segms[segmentID - 1];
    // Return true if the values match, false otherwise
    return segmentFileValue == rbcSegmentFileValue;
}


/* Serves a request from a train (TRENO) for authorization to advance to a new position.
The function takes in a single parameter: an integer representing the file descriptor of the client socket connected to the TRENO.
The function receives a message from the TRENO via the client socket, parses the message to obtain the TRENO's ID, current position, and next position, decides whether to authorize the TRENO to advance to the next position based on the status of the next position in a shared memory data structure and the status of the current and next positions, sends the authorization decision to the TRENO via the client socket, closes the client socket, and updates the shared memory data structure and an RBC log file with information about the TRENO's authorization request. */

void requestS(int client_fd) {
    // Create shared memory (SHM)
    const int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if(shm_fd == -1) throwError("requestS: failed to create SHM");
    // Create rbcData shared memory between RBC and its children
    rbcData_t *rbcData = (rbcData_t*)mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(rbcData == MAP_FAILED) throwError("Failed to map rbcData to shared memory");
    // Receive message from TRENO
    char buffer[32] = { 0 };
    if(recv(client_fd, buffer, sizeof(buffer), 0) == -1) throwError("Failed to receive message from TRENO");
    char *msg_read = strdup(buffer);
    const char str_sep = '~';
    // Get TRENO ID
    int trainNum;
    char *tmp_str = strsep(&msg_read, &str_sep);
    sscanf(tmp_str, "%d", &trainNum);
    // Get TRENO current position
    char *currPos = strsep(&msg_read, &str_sep);
    // Get TRENO next position
    char *nextPos = strsep(&msg_read, &str_sep);
    // Check if currPos and nextPos are stations or segments
    const bool currStation = stationVerifier(currPos);
    const bool nextStation = stationVerifier(nextPos);
// Get position IDs
int currID, nextID;
if(currStation) sscanf(currPos, "S%d", &currID);
else sscanf(currPos, "MA%d", &currID);
if(nextStation) sscanf(nextPos, "S%d", &nextID);
else sscanf(nextPos, "MA%d", &nextID);
// RBC decides if TRENO can advance
const bool nextStaFree = (nextStation || !rbcData->segms[nextID - 1]);
const bool nestSegSta = segmStatusChecker(rbcData, nextID, nextPos, nextStation);
const bool currStaCorrect = segmStatusChecker(rbcData, currID, currPos, currStation);
const bool auth = nextStaFree && nestSegSta && currStaCorrect;
// RBC sends authorization to TRENO
if(send(client_fd, &auth, sizeof(auth), 0) == -1) throwError("Failed to send authorization to TRENO");
// TRENO has been executed 
close(client_fd);
    // rbcData updates on requests
    if(auth) {
        if(nextStation) {
            rbcData->stations[nextID - 1]++;
            // TRENO reached destination
            kill(getppid(), SIGUSR1);
        }
        else rbcData->segms[nextID - 1] = true;
        if(currStation) rbcData->stations[currID - 1]--;
        else rbcData->segms[currID - 1] = false;
    }
    // RBC updates log
    rbcLogUpdate(trainNum, currPos, nextPos, auth);
    // Remove access to shared memory
    if((munmap(rbcData, SHM_SIZE)) == -1) throwError("requestS: unmapping shared memory failed");
    // Close shared memory file descriptor
    close(shm_fd);
// Free allocated memory
    free(tmp_str);
    free(msg_read);
    exit(EXIT_SUCCESS); 
}

// RBC MAIN
/* This is the main function of the RBC program. It creates a shared memory segment and server socket, initializes the shared memory data structure, sets a signal handler, checks for empty paths in the shared memory data, removes the RBC log file if it exists, and runs the RBC server. */

int main(void) {
    signal(SIGUSR1, signalHandler); // Set signal handler for SIGUSR1
    signal(SIGUSR2, signalHandler2); // Set signal handler for SIGUSR2
    printf("RBC Execution initialized.\n");
    const int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1) throwError("Error opening shared memory");
    ftruncate(shm_fd, SHM_SIZE);
    rbcData_t *rbcData = (rbcData_t*)mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(rbcData == MAP_FAILED) throwError("Error mapping shared memory");
    rbcDataInit(rbcData);
    unlink(RBC_LOG); // Remove RBC log file if it exists
    const int server_fd = rbcServerSocket();  // Create server socket

    // Server function for the RBC process.
    while (true) {
        // Client address
        struct sockaddr_un client_addr;
        struct sockaddr *client_addr_ptr = (struct sockaddr*)&client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd;
        pid_t pid;
        // RBC waits for requests from TRENO processes
        printf("RBC Server waiting for TRENO requests.\n");
        switch (client_fd = accept(server_fd, client_addr_ptr, &client_len)) {
            case -1:
                throwError("Error accepting TRENO request");
                break;
            default:
                // TRENO is appointed a child process of RBC when a request is taken
                switch (pid = fork()) {
                    case -1:
                        throwError("Error creating child process");
                        break;
                    case 0:
                        // Child process handles request
                        close(server_fd);
                        requestS(client_fd);
                        break;
                    default:
                        // Parent process closes client file descriptor
                        close(client_fd);
                        break;
                }
                break;
        }
    }
    return EXIT_SUCCESS;
}
