# Rail-Control-System
Movement Authority ERTMS/ETCS simulation using processes, pipes and sockets in C

Movement Authority ERTMS/ETCS LV 1 LV 2 Simulation

Welcome to the Movement Authority ERTMS/ETCS LV 1 LV 2 Simulation project! This project aims to create a simulation of how various trains cross track intersections while ensuring there are no collisions. It allows only one train to cross any railway segment at a time, simulating real-world railway safety protocols.

Project Structure

The project is divided into five main elements:

PADRE_TRENI: This process is responsible for creating 16 files representing the maximum segments. These files are configured with read/write access and are marked with 1 or 0 to indicate whether they are occupied or not.
TRENO: There are five processes representing different trains.
REGISTRO: This component serves as a registry for the itineraries that each train will follow.
RBC (Radio Block Center): Manages the AF_UNIX server socket. When called by executing the program in ETC2 mode, the RBC retrieves itineraries from REGISTRO and manages the maximum segments. It creates a process for each request it receives.
Signal: Manages the SIGUSR1 and SIGUSR2 signals that the processes use before terminating.
Execution Modes

The program can be executed in two ways:

ETCS1: Execute the program through the shell.
ETCS2: Execute the program through two different shells. The first mode is by calling ETCS2 directly, and the second is by specifying ETCS2 RBC.
Operating Systems - Project 2

Main Execution Flow
The program's input arguments are taken into account, and the program is executed accordingly based on the specified parameters (type of itinerary, type of execution, RBC).

If the program is run as ETCS1, it will be executed with PROCESSO_PADRE, which creates the five train processes (PROCESSI_TRENI), and PROCESSO_REGISTRO provides the itinerary to each of the trains.
If the program is run using the ETCS2 or ETCS2 RBC settings, the program will be executed with RBC as a server socket, which handles the itinerary from the registry for every train. The handling of requests from the different train processes is done in parallel.
Makefile

A Makefile is provided for the assembly of the program. It sets the executable files ready and provides a make clean command to delete them accordingly when the execution is over.

Operating Systems - Project 3

Execution of the Program
To execute the program, follow these steps:

Run the Makefile with the following command-line:
go
Copy code
make
This will create the bin and obj directories with the UNIX executable files.
Start the program using the following command:
arduino
Copy code
./run.sh
This command can take two optional parameters:
-e: Sets the ETC mode in which the program will run (1 or 2). If no argument is specified, it will run in mode 1 by default.
-m: Sets the MAPPA in which the program will run (1 or 2). If no argument is specified, it will run in mode 1 by default.
-h: Shows the available command-line arguments.
When executing in ETC1 mode (./run.sh -m 1/2), REGISTRO sends the itineraries directly to each TRENO process.
When executing in ETC2 mode (./run.sh -e 2 -m 1/2), the RBC manages the itineraries and handles requests from different train processes in parallel.
Operating Systems - Project 4

Logs
As the program is executed, a log is updated for each train (T1, T2, T3, T4, T5). This log includes each step of the train until it reaches its destination, showing the current segment in each step, the next segment, and the date and time.

Feel free to explore the code and documentation in this repository to gain a deeper understanding of the project. If you have any questions or need further assistance, please don't hesitate to reach out to the project maintainers.
