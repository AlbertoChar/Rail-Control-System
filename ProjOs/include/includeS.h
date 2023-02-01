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

#include <sys/stat.h>

#include "../include/includeF.h"

#pragma once

void signalHandler(int sign);
void signalHandler2(int sign);
void signalDone(char *paths[N_TRAINS]);