#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>

#include "../include/includeF.h"
#include "../include/includeL.h"
#include "../include/includeS.h"

// Global constants
const char *noPosition = "--";
const char pathSeparator = '-';

// segmUpdate modifies the status of a segment by updating the corresponding segment file with the new status.
// Parameters:
//   - segmNum: the number of the segment whose status is being modified
//   - empty: a boolean indicating whether the segment is empty or not.
void segmUpdate(const int segmNum, const bool empty) {
    // Create the filename for the segment file
    char filename[32];
    sprintf(filename, SEGM_FORMAT, segmNum);
    // Open the file for reading and writing
    int fd;
    if((fd = open(filename, O_RDWR, 0666)) == -1) {
        throwError("Failed to open segment file");
    }
    // Get the file status
    struct stat fs;
    if((fstat(fd, &fs)) == -1) {
        throwError("Failed to get file status");
    }
    // Map the file into memory
    char *mappedFile = (char *)mmap(NULL, fs.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(mappedFile == MAP_FAILED) {
        throwError("Failed to map file into memory");
    }
    // Update the status of the segment in the mapped file
    // 0 when the segment is free, 1 when it is occupied
    const char fileContent = empty ? '0' : '1';
    memcpy(mappedFile, &fileContent, sizeof(char));
    // Unmap the file from memory
    if((munmap(mappedFile, fs.st_size) == -1)) {
        throwError("Failed to unmap file from memory");
    }
    close(fd);
}

// rbcConnect establishes a connection between a train process and the RBC (Radio Block Center) process.
// Parameters:
//   - trainNum: the number of the train process that is establishing the connection
// Returns: the file descriptor of the socket used to establish the connection
int rbcConnect(int trainNum) {
    // Server address
    struct sockaddr_un server_addr;
    struct sockaddr* server_addr_ptr = (struct sockaddr*) &server_addr;
    socklen_t server_len = sizeof(server_addr);
    // Socket creation
    int client_fd;
    if((client_fd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL)) == -1) {
        throwError("Failed to create socket");
    }
    // Socket options
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SERVER_NAME);
    // TRENO tries to connect to RBC
    printf("TRENO %d: Trying to form a connection to RBC.\n", trainNum);
    int connected;
    do {
        connected = connect(client_fd, server_addr_ptr, server_len);
        if (connected == -1) {
            sleep(1);
        }
    } while(connected == -1);
    printf("TRENO %d Connection to RBC established.\n", trainNum);
    return client_fd;
}

// Request from RBC to proceed
// This function sends a message to RBC with the train's ID, current position, and next position
// It then receives and returns a boolean indicating whether RBC approves the train to proceed
bool advanceAppr(int trainNum, char *currPos, char *nextPos) {
    // Connection to RBC
    const int client_fd = rbcConnect(trainNum);
    // Message to RBC
    // Calculate the length of the message to send to RBC
    const int messageLength =
        sizeof(trainNum) + (strlen(currPos) * sizeof(char)) +
        (strlen(nextPos) * sizeof(char)) + (3 * sizeof(char));
    // Allocate memory for the message and format it with the train's ID, current position, and next position
    char *rbcMessage = (char *)malloc(messageLength);
    snprintf(rbcMessage, messageLength, "%d~%s~%s", trainNum, currPos, nextPos);
    // Send the message to RBC
    if(send(client_fd, rbcMessage, messageLength, 0) == -1) {
        throwError("Failed to send message to RBC");
    }
    printf("TRENO %d ID message %s sent to RBC.\n", trainNum, rbcMessage);
    // Receive authorization from RBC
    bool auth;
    if(recv(client_fd, &auth, sizeof(auth), 0) == -1) {
        throwError("Failed to receive authorization from RBC");
    }
    printf("TRENO %d Authorization %d received from RBC.\n", trainNum, auth);
    // Close the connection and free the memory for the message
    close(client_fd);
    free(rbcMessage);
    // Return the authorization received from RBC
    return auth;
}


// Proceeding to next position request for TRENO
bool canProceed(const int etcs, int trainNum, char *currPos, char *nextPos, const bool station) {
    // When in ETCS2 then TRENO must ask RBC
    if(etcs == 2 && !advanceAppr(trainNum, currPos, nextPos)) return false;
    // Check that next position is a station
    if(station) return true;
    // If next position is a segment, then check if free
    return isSegmentFree(nextPos);
}

// Bool for TRENO advancement
bool moveForward(const int etcs, int trainNum, char *currPos, char *nextPos) {
    // True when next position is a station
    const bool currStation = stationVerifier(currPos);
    const bool nextStation = stationVerifier(nextPos);
    // if TRENO cant proceed, waits for next iteration
    if(!canProceed(etcs, trainNum, currPos, nextPos, nextStation)) return false;
    int segmNum;
    if(!nextStation) {
        // Next position number
        sscanf(nextPos, "MA%d", &segmNum);
        // Next position occupation
        segmUpdate(segmNum, false);
    }
    if(!currStation) {
        // Current position number
        sscanf(currPos, "MA%d", &segmNum);
        // Current position liberation
        segmUpdate(segmNum, true);
    }
    return true;
}

// Itinerary request
// This function connects to the registro pipe for the given train and receives the itinerary from REGISTRO
char* getIt(const int trainNum) {
    // Connects to registro pipe for the given train
    const int rPipe = connectToFifo(PIPE_FORMAT, trainNum);
    // Read the itinerary from the pipe into a buffer
    char buffer[128];
    if((read(rPipe, buffer, sizeof(buffer))) == -1) {
        throwError("Failed to read from registro pipe");
    }
    // Close the connection to the registro pipe
    if((close(rPipe)) == -1) {
        throwError("Failed to close registro pipe connection");
    }
    // Return a copy of the itinerary from the buffer
    return strdup(buffer);
}

/* main function for the TRENO process. It does the following:
- Checks that the correct number of arguments have been passed
- Initializes the trainNum and etcs variables from the arguments passed to the function
- Prints an execution start message
- Receives the itinerary for the train from REGISTRO
- If no itinerary is received, updates the log file with the "no position" value and terminates execution
- Gets the current position of the train and the next position
- Loops through the itinerary until the end is reached, waiting for permission to move to the next position and updating the current position
- Updates the log file for the last iteration
- Frees dynamically allocated memory
- Prints an execution termination message */

int main(int argc, char *argv[]) {
    if(argc != 3) {
        throwError("Invalid number of arguments");
    }
// Initialize variables
int trainNum, etcs;
sscanf(argv[1], "%d", &trainNum); // Convert first argument to int and store it in trainNum
sscanf(argv[2], "%d", &etcs); // Convert second argument to int and store it in etcs
printf("TRENO %d Began execution.\n", trainNum); // Print execution start message
char *trainItinerary = getIt(trainNum); // Get the itinerary for the train
// If no itinerary is received, terminate execution
if(!strcmp(trainItinerary, noPosition)) {
    logUpdate(trainNum, (char*)noPosition, (char*)noPosition);
    free(trainItinerary);
    exit(EXIT_SUCCESS);
}
// Get the current position of the train and the next position
char *currPos = strsep(&trainItinerary, &pathSeparator);
char *nextPos;
// Loop through the itinerary until the end is reached
while((nextPos = strsep(&trainItinerary, &pathSeparator))) {
    // Update the log file for each iteration
    logUpdate(trainNum, currPos, nextPos);
    // Wait for permission to move to the next position
    do {
        sleep(2);
        printf("TRENO %d Current position: %s, requesting permission to proceed to next position: %s.\n", trainNum, currPos, nextPos);
    } while(!moveForward(etcs, trainNum, currPos, nextPos));
    // Update the current position
    currPos = strdup(nextPos);
}
// Update the log file for the last iteration
logUpdate(trainNum, currPos, (char*)noPosition);
// Free dynamically allocated memory
free(currPos);
free(nextPos);
printf("TRENO %d Execution terminated.\n", trainNum);
//SIGUSR1 signal to PADRE_TRENI
    printf("Sending SIGUSR1 to PADRE_TRENI, pid: %d\n", getppid());
    kill(getppid(), SIGUSR1);
return EXIT_SUCCESS;
}
