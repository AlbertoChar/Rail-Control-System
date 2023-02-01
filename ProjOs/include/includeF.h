#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <signal.h>

// MACROS
#define N_TRAINS 5
#define N_STATIONS 8
#define N_SEGM 16
#define N_MAPS 2
#define N_ETCS 2
#define DEFAULT_PROTOCOL 0
#define N_RBC_PIPE 0
#define SERVER_NAME "/tmp/rbc_server"
#define PIPE_FORMAT "/tmp/reg_pipe%d"
#define SEGM_FORMAT "/tmp/MA%d.txt"
#define SHM_SIZE 512
#define SHM_NAME "rbc_data"
#define RBC_LOG "log/RBC.log"

#pragma once

int rbcPid;
int connectToFifo(const char*, int);
char* getCurrTime();

void throwError(const char*);
void trenoWait();
void logUpdate();

bool stationVerifier(char *str);
bool isSegmentFree(char *);

// TYPEDEFS
typedef struct cmd_args {
    int etcs;
    bool rbc;
    int mappa;
} cmd_args;
typedef struct itin {
    char *start;
    char *end;
    char *path;
} itin;
typedef struct rbcData_t {
    bool segms[N_SEGM];
    int stations[N_STATIONS];
    char *paths[N_TRAINS];
} rbcData_t;
typedef itin railMaps[N_TRAINS];