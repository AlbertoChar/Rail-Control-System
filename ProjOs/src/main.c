#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../include/includeF.h"

// Global Constants
const char *registro_exec = "./bin/registro";
const char *rbc_exec = "./bin/rbc";
const char *padre_treni_exec = "./bin/padre_treni";
const char *log_dir = "log";

// execRegistro creates the REGISTRO and PADRE_TRENI processes. If the ETCS argument is 1, it also unlinks
// the RBC_LOG file.
// Parameters:
//   - args: a struct containing the command line arguments passed to the main function, RBC PID for the SIGUSR2 signal is passed as an argument as well.
// The function creates the REGISTRO and PADRE_TRENI processes and waits for them to finish
// execution before returning
void execRegistro(const cmd_args args) {
  // If ETCS is 1, unlink RBC_LOG
  if (args.etcs == 1) unlink(RBC_LOG);
  // Convert ETCS and map arguments to strings
  char etcs_str[4], map_str[4];
  sprintf(etcs_str, "%d", args.etcs);
  sprintf(map_str, "%d", args.mappa);
  // REGISTRO process creation
  pid_t pid;
  switch (pid = fork()) {
    case -1:
      throwError("Fork failed to create REGISTRO process");
      break;
    case 0:
      // Execute REGISTRO process
      switch (execl(registro_exec, registro_exec, etcs_str, map_str, NULL)) {
        case -1:
          // Throw error if execl fails to execute REGISTRO process
          throwError("Execl failed to execute REGISTRO process");
          break;
      }
      break;
    default:
      printf("REGISTRO process created\n");
      break;
  }
  // PADRE_TRENI process creation
  char arg[256]; // RBC PID argument Variable
    switch (pid = fork()) {
    case -1:
      // Throw error if fork fails to create PADRE_TRENI process
      throwError("Fork failed to create PADRE_TRENI process");
      break;
    case 0:
      // Execute PADRE_TRENI process
      sprintf(arg, "%d", rbcPid); // Assignment of RBCPID
      switch (execl(padre_treni_exec, padre_treni_exec, etcs_str, arg, NULL)) {
        case -1:
          // Throw error if execl fails to execute PADRE_TRENI process
          throwError("Execl failed to execute PADRE_TRENI process");
          break;
      }
      break;
    default:
      printf("PADRE_TRENI process created\n");
      break;
  }
  // Main process waiting for REGISTRO and PADRE_TRENI to finish execution
  trenoWait();
  printf("REGISTRO, PADRE_TRENI: end of execution\n");
}


/* MAIN
 Read arguments passed as input from the command line.
 Create a log directory.
 Pass execution to RBC process if ETCS2 and RBC.
 otherwise execute default PADRE_TRENI and REGISTRO */
 
int main(int argc, char *argv[]) {
    // Initialize all arguments to 0
    cmd_args args;
    args.etcs = args.mappa = args.rbc = 0;
    for (int i = 1; i < argc; i++) {
        char* currentArg = argv[i];
        // Check if the current argument is an ETCS argument
        if (strlen(currentArg) > 4 && !strncmp("ETCS", currentArg, 4)) {
            // Parse the ETCS argument
            if (sscanf(currentArg, "ETCS%d", &args.etcs) != 1) {
                throwError("Invalid ETCS argument");
            }
        }
        // Check if the current argument is a MAPPA argument
        else if (strlen(currentArg) > 5 && !strncmp("MAPPA", currentArg, 5)) {
            // Parse the MAPPA argument
            if (sscanf(currentArg, "MAPPA%d", &args.mappa) != 1) {
                throwError("Invalid MAPPA argument");
            }
        }
        // Check if the current argument is an RBC argument
        else if (!strcmp("RBC", currentArg)) {
            // Set the RBC flag to true
            args.rbc = true;
        }
        else if (atoi(currentArg) != 0){
            rbcPid = atoi(currentArg);
            printf("MAIN RBC PID: %d\n", rbcPid);
        }
        // If the current argument is none of the above, it is invalid
        else {
            throwError("Invalid argument");
        }
    }
    // Check if the parsed argument values are valid
    if (!(args.etcs > 0 && args.etcs <= N_ETCS) || !(args.mappa > 0 && args.mappa <= N_MAPS)) {
        throwError("Invalid values for ETCS or MAPPA arguments");
    }
    // Print parsed arguments
    printf("ETCS%d MAPPA%d RBC=%d\n", args.etcs, args.mappa, args.rbc);
    // Create log directory
    mkdir(log_dir, 0777);
    // Check if ETCS is 2 and RBC flag is set
    if (args.etcs == 2 && args.rbc) {
        // Execute RBC process
        if (execl(rbc_exec, rbc_exec, NULL) == -1) {
            throwError("Execl failed to execute RBC process");
        }
    }
    else {
        // Execute REGISTRO and PADRE_TRENI processes
        execRegistro(args);
    }
    // Return success
    return EXIT_SUCCESS;
}