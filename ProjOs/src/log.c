#include "includeF.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

//Log updates

// logUpdate updates the log file for a train with the current and next positions of the train, as well as the current time.
// Parameters:
//   - trainNum: the number of the train whose log file is being updated
//   - currPos: the current position of the train
//   - nextPos: the next position of the train
void logUpdate(int trainNum, char *currPos, char *nextPos) {
    // Open the log file for writing, creating it if it doesn't exist and truncating it to zero length if it does exist
    static int oFlags = O_CREAT | O_WRONLY | O_TRUNC;
    char filename[16];
    sprintf(filename, "log/T%d.log", trainNum);
    int fd;
    if((fd = open(filename, oFlags, 0666)) == -1) {
        throwError("Failed to open log file");
    }
    // First function call - oFlags setup
    oFlags = O_CREAT | O_WRONLY | O_APPEND;
    // Line to write on file
    char writeLine[64] = { 0 };
    sprintf(writeLine, "[Current: %s], [Next: %s], %s", currPos, nextPos, getCurrTime());
    const size_t lineLength = strlen(writeLine) * sizeof(char);
    if((write(fd, writeLine, lineLength)) == -1) {
        throwError("Failed to write to log file");
    }
    close(fd);
}

// RBC log update
// Updates the RBC log file with a line containing information about a train's authorization request.
void rbcLogUpdate(const int trainNum, char *currPos, char* nextPos, const bool auth) {
    // Open RBC log file for writing
    int fd;
    if ((fd = open(RBC_LOG, O_CREAT | O_WRONLY | O_APPEND, 0666)) == -1) {
        // Throw error if file cannot be opened
        throwError("Failed to open RBC log file for writing");
    }
    // Line to be written to the file
    char writeLine[128] = { 0 };
    // Get the string to use for the authorization value
    char auth_str[3];
    if (auth) {
        strcpy(auth_str, "SI");
    } else {
        strcpy(auth_str, "NO");
    }
    // Format the line to be written to the file
    sprintf(writeLine,
            "[TRENO authorization request: T%d], [Current: %s], [Next: %s], [Authorized: %s], %s",
            trainNum, currPos, nextPos, auth_str, getCurrTime());
    // Size of bytes to be written to the file
    const size_t lineLength = strlen(writeLine) * sizeof(char);
    // Write line to file
    if (write(fd, writeLine, lineLength) == -1) {
        // Throw error if writing fails
        throwError("Failed to write to RBC log file");
    }
    close(fd);
}