#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/includeF.h"


// This function checks whether a given string is a valid station identifier.
// A valid station identifier is a string that starts with the letter 'S' followed by a positive integer.
// For example, "S1", "S2", etc. are all valid station identifiers.
// If the given string is a valid station identifier, the function returns true. Otherwise, it returns false.
bool stationVerifier(char *str) {
    // Initialize a string to store the integer part of the station identifier
    char id_str[8] = { 0 };
    // Initialize a variable to store the integer value of the station identifier
    int stationNum;
    // If the first character of the string is not 'S', it cannot be a valid station identifier
    if (str[0] != 'S') return false;
    // Copy the integer part of the station identifier into the id_str string
    strcpy(id_str, str + 1);
    // Parse the integer value of the station identifier from the id_str string
    sscanf(id_str, "%d", &stationNum);
    // If the integer value of the station identifier is not a positive integer between 1 and N_STATIONS (inclusive),
    // it is an invalid station identifier
    if (stationNum <= 0 || stationNum > N_STATIONS) {
        // Throw an error to indicate that the station identifier is invalid
        throwError("Station identifier error");
    }
    // If the station identifier is valid, return true
    return true;
}

/* connectToFifo attempts to connect to the pipe specified by the given filename formatted using the given
 train number and returns the file descriptor for the pipe. If the connection request is unsuccessful,
 the function will retry every 1 second until a connection is established.
 Parameters:
   - formatPipeC: a string containing a format specifier for the desired pipe filename
   - trainNum: the train number to be used in formatting the desired pipe filename
 Returns: the file descriptor for the connected pipe */
 
int connectToFifo(const char* formatPipeC, int trainNum) {
    // Create the filename for the pipe using the provided format string and train number
    char filename[16];
    sprintf(filename, formatPipeC, trainNum);
    // Print a message indicating that a connection request is being made
    if (trainNum == N_RBC_PIPE) {
        printf("RBC Connection request to %s.\n", filename);
    } else {
        printf("TRENO %d Connection request to %s.\n", trainNum, filename);
    }
    // Attempt to open the pipe for reading
    int fd;
    do {
        fd = open(filename, O_RDONLY);
        if (fd == -1) {
            // Sleep for 1 second if the pipe could not be opened
            sleep(1);
        }
    } while (fd == -1);
    // Print a message indicating that the connection was successful
    if (trainNum == N_RBC_PIPE) {
        printf("RBC Connected to %s (fd: %d) succeeded.\n", filename, fd);
    } else {
        printf("TRENO %d Connection to %s (fd: %d) succeeded.\n", trainNum, filename, fd);
    }
    // Return the file descriptor for the pipe
    return fd;
}

// This function waits for all treno processes to terminate.
// It continually calls the waitpid function until it returns a value less than or equal to 0,
// indicating that there are no more child processes to wait for.
// Between each call to waitpid, the function sleeps for 1 second to avoid busy waiting.
void trenoWait() {
    // Repeatedly call waitpid until it returns a value less than or equal to 0
    while (waitpid(0, NULL, 0) > 0) {
        // Sleep for 1 second between each call to waitpid to avoid busy waiting
        sleep(1);
    }
}


bool isSegmentFree(char *segm) {
    // Create the filename for the segment file
    char filename[16];
    sprintf(filename, "/tmp/%s.txt", segm);
    // Open the segment file for reading
    int fd;
    if ((fd = open(filename, O_RDONLY, 0444)) == -1) {
        // Throw an error if the file could not be opened
        throwError("Error opening segment file, try again");
    }
    // Get the file info
    struct stat fs;
    if ((fstat(fd, &fs)) == -1) {
        // Throw an error if the file info could not be retrieved
        throwError("Error getting file info");
    }
    // Map the file into memory
    char *mappedFile = (char *)mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mappedFile == MAP_FAILED) {
        // Throw an error if the file could not be mapped into memory
        throwError("Error mapping file into memory");
    }
    // Read the file
    const char fileContent = *mappedFile;
    // Unmap the file from memory
    if ((munmap(mappedFile, fs.st_size) == -1)) {
        // Throw an error if the file could not be unmapped from memory
        throwError("Error unmapping file from memory");
    }
    // Close the file
    close(fd);
    // Check the file content
    if (fileContent != '0' && fileContent != '1') {
        // Throw an error if the file content is invalid
        throwError("Invalid file content");
    }
    // Return whether the segment is free
    return fileContent == '0';
}

char* getCurrTime() {
    const time_t now = time(NULL);
    const struct tm *time_ptr = localtime(&now);
    return asctime(time_ptr);
}

// Error management
void throwError(const char* msg) {
    perror(msg);
    if(!errno) exit(EXIT_FAILURE);
    exit(errno);
}