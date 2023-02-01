#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/includeF.h"



// SIGNAL HANDLERS

// Called each time a train arrives at its destination. When all TRENO processes terminate, RBC terminates.
void signalHandler(int sign) {
    printf("TRENO process terminated.\n");
        // Repeatedly call waitpid until it returns a value less than or equal to 0
        if (waitpid(0, NULL, 0) <= 0) {
            // When in ETC 2 mode, use a SIGUSR signal to terminate RBC before terminating
            if (rbcPid != 0) {
                printf("Sending SIGUSR2 to RBC, pid: %d\n", rbcPid);
                kill(rbcPid, SIGUSR2);
            }
            printf("TRENO processes terminated.\n");
            printf("REGISTRO, PADRE_TRENI: end of execution\n");
            // Delete the segment files
            char filename[32];
            for (int i = 1; i <= N_SEGM; i++) {
                sprintf(filename, SEGM_FORMAT, i);
                unlink(filename);
            }
            exit(EXIT_SUCCESS);
        } else {
            // Sleep for 1 second between each call to waitpid to avoid busy waiting
            sleep(1);
        }
}

// Signal Handler for SIGUSR2
void signalHandler2(int sign){
    printf("SIGUSR2 from Padre Treni to RBC, terminating RBC\n");
    // Remove shared memory and servers
    if (shm_unlink(SHM_NAME) == -1) {
        perror("Error removing shared memory\n");
    } else {
        printf("Shared memory %s removed.\n", SHM_NAME);
    }
    if (unlink(SERVER_NAME) == -1) {
        perror("Error closing server\n");
    } else {
        printf("RBC Server %s closed.\n", SERVER_NAME);
    }
    // Print message and exit
    printf("RBC Execution terminated.\n");
    exit(EXIT_SUCCESS);
}