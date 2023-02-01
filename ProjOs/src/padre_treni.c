#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "../include/includeF.h"
#include "../include/includeS.h"

const char* treno_exec = "./bin/treno";


// createSegm modifies the state of a segment by creating or truncating its corresponding segment file and
// writing the initial state of the segment to the file.
// Parameters:
//   - segmNum: the number of the segment whose state is being modified

void createSegm(const int segmNum) {
    // Create the filename for the segment file
    char filename[32];
    sprintf(filename, SEGM_FORMAT, segmNum);
    // Open the file for writing, creating it if it doesn't exist, and truncating it to zero length if it does exist
    int fd;
    if((fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0666)) == -1) {
        // If the file couldn't be opened, throw an error
        throwError("createSegm: opening Error");
    }
    // Write the initial state of the segment to the file
    if(write(fd, "0", sizeof(char)) == -1) {
        // If the write operation failed, throw an error
        throwError("createSegm: writing Error");
    }
    // Write an empty character to the file
    if(write(fd, "", sizeof(char)) == -1) {
        // If the write operation failed, throw an error
        throwError("createSegm: writing Error");
    }
    // Close the file
    close(fd);
}

// main is the entry point for the PADRE_TRENI process. It initializes the segment files, creates the TRENO processes, and waits for them to finish execution before deleting the segment files and returning.
// Returns: 0 on success, a non-zero value on failure

int main(int argc, char *argv[]) {
    signal(SIGUSR1, signalHandler);
    printf("PADRE_TRENI Execution initialized.\n");
    // Check that the correct number of arguments was passed to the main function
    if(argc != 3) throwError("PADRE_TRENI arguments invalid");
    // Creates N_SEGM file, each one associated to a segment
    for(int i=1; i<=N_SEGM; i++) createSegm(i);
    char tr_id_str[4];
    pid_t pid;
    // Creates N_TRAINS processes, each one associated to a train
    for(int i=1; i<=N_TRAINS; i++) {
        if((pid = fork()) == 0) {
            // Convert the train number to a string and execute the TRENO process
            sprintf(tr_id_str, "%d", i);
            execl(treno_exec, treno_exec, tr_id_str, argv[1], NULL);
            throwError("PADRE_TRENI execl error");
        }
        else if(pid == -1) {
            // Throw an error if the fork fails to create the TRENO process
            throwError("PADRE_TRENI fork error");
        }
        printf("PADRE_TRENI created process for TRENO %d\n", i);
    }
    // Process PADRE_TRENO waiting for TRENO
    trenoWait();
    printf("TRENO processes terminated.\n");
    rbcPid = atoi(argv[2]);
    // When in ETC 2 mode, use a SIGUSR signal to terminate RBC before terminating
    if(rbcPid != 0) {
        printf("Sending SIGUSR2 to RBC, pid: %d\n", rbcPid);
        kill(rbcPid, SIGUSR2);
    }
    //Delete the segment files
    char filename[32];
    for(int i=1; i<=N_SEGM; i++) {
        sprintf(filename, SEGM_FORMAT, i);
        unlink(filename);
    }
    return EXIT_SUCCESS;
}