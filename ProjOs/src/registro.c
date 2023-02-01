#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/stat.h>

#include "../include/includeF.h"
#include "../include/includeM.h"

// Creation and opening of REGISTRO pipe
// This function creates and opens a pipe with a filename based on the given pipe number and format string
int pipeOpen(const char *formatPipeC, const int pipeNum) {
    // Generate the filename for the pipe based on the given pipe number and format string
    char filename[16];
    sprintf(filename, formatPipeC, pipeNum);
    // Remove the pipe if it already exists, then create a new one with the specified filename and permissions
    unlink(filename);
    mkfifo(filename, 0666);
    // Open the pipe in write-only mode and store the file descriptor
    int fd;
    if((fd = open(filename, O_WRONLY)) == -1) throwError("Failed to open pipe");
    // Return the file descriptor for the opened pipe
    return fd;
}

// FD closure (and consecuently REGISTRO pipe elimination)
// This function closes the given file descriptor and removes the pipe with the corresponding filename
void pipeClose(const char *formatPipeC, const int fdPipe, const int pipeNum) {
    // Close the given file descriptor
    close(fdPipe);
    // Generate the filename for the pipe based on the given pipe number and format string
    char filename[16];
    sprintf(filename, formatPipeC, pipeNum);
    // Remove the pipe with the generated filename
    unlink(filename);
}

// Waits for TRENO request 
// This function sends the given itinerary to the TRENO process with the given number through a pipe
void itineraryToTrains(int trainNum, const itin itin) {
    // Initialize a buffer for the itinerary message
    char buffer[128] = { 0 };
    // Convert the itinerary to a string
    sprintf(buffer, "%s-%s-%s", itin.start, itin.path, itin.end);
    // Calculate the length of the message string
    const int messageLength = (strlen(buffer) + 1) * sizeof(char);
    // Create and open the pipe to the TRENO process
    const int registroPipe = pipeOpen(PIPE_FORMAT, trainNum);
    // Write the message string to the pipe
    if(write(registroPipe, buffer, messageLength) == -1) throwError("Failed to write itinerary message to pipe for TRENO");
    printf("REGISTRO Sent itinerary %s of TRENO %d.\n", buffer, trainNum);
    // Close the pipe and remove it
    pipeClose(PIPE_FORMAT, registroPipe, trainNum);
    exit(EXIT_SUCCESS);
}

// Itineraries to TRENO from REGISTRO
// This function sends the itineraries in the given map to the corresponding TRENO processes
void mapToTrains(const railMaps map) {
    // Loop through the map and send an itinerary to each TRENO process
    pid_t pid;
    for(int i=1; i<=N_TRAINS; i++) {
        // Create a child process for sending the itinerary to the TRENO process
        if((pid = fork()) == 0) itineraryToTrains(i, map[i - 1]);
        else if(pid == -1) throwError("Failed to create child process for sending itinerary to TRENO");
    }
    // Wait for all child processes to complete
    trenoWait();
    exit(EXIT_SUCCESS);
}

/* Main function for REGISTRO process.
  This function receives two arguments:
    - argv[1]: the ETCS value (either 1 or 2)
    - argv[2]: the index of the map to use (1 or 2)
  It performs the following actions:
    - Validates the number of arguments received.
    - Parses the ETCS and map index from the arguments.
    - If ETCS value is 2, creates a child process to send the map to RBC.
    - Creates a child process to send the itineraries to the TRENO processes.
    - Waits for the child processes to finish. */

int main(int argc, char *argv[]) {
    if(argc != 3) throwError("Invalid number of arguments in REGISTRO");
    int etcs, n_map;
    sscanf(argv[1], "%d", &etcs);
    sscanf(argv[2], "%d", &n_map);

    if(etcs == 2) {
        const char *sep = "-";
        char buffer[512] = { 0 };
        for(int i=0; i<N_TRAINS; i++) {
            strcat(buffer, maps[n_map - 1][i].start);
            strcat(buffer, sep);
            strcat(buffer, maps[n_map - 1][i].path);
            strcat(buffer, sep);
            strcat(buffer, maps[n_map - 1][i].end);
            if(i != (N_TRAINS - 1)) strcat(buffer, "~");
        }

        const int messageLength = (strlen(buffer) + 1) * sizeof(char);
        const int registroPipe = pipeOpen(PIPE_FORMAT, N_RBC_PIPE);
        if(write(registroPipe, buffer, messageLength) == -1) throwError("Failed to write map message to pipe");
        printf("Map %s sent to RBC.\n", buffer);
        pipeClose(PIPE_FORMAT, registroPipe, N_RBC_PIPE);
    }

    pid_t pid;
    if((pid = fork()) == 0) mapToTrains(maps[n_map - 1]);
    else if(pid == -1) throwError("Failed to create child process for sending itinerary to TRENO");

    trenoWait();
    printf("REGISTRO Execution terminated.\n");
    return EXIT_SUCCESS;
}
