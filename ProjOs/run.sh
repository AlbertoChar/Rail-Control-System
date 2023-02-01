#!/bin/bash

# Set default values for the ETCS and MAPPA options
etcs=1          # ETCS1
mappa=1         # MAPPA1

# Define a usage message to display when the -h option is used
usage_msg="Usage: $(basename "$0") [-e arg] [-m arg]"

# Process command line options
while getopts ":e:m:h" flags; do
    # Check the value of the flags variable
    if [[ $flags == "e" ]]; then
        # If the -e option is used, set the etcs variable to the value of OPTARG
        etcs=${OPTARG}
    elif [[ $flags == "m" ]]; then
        # If the -m option is used, set the mappa variable to the value of OPTARG
        mappa=${OPTARG}
    elif [[ $flags == "h" ]]; then
        # If the -h option is used, display the usage message and exit
        echo "$usage_msg"
        exit 0
    else
        # If an option requires an argument but none is provided, or an invalid option is used, display an error message and exit
        if [[ $flags == ":" ]]; then
            echo -e "Option -$OPTARG requires an argument.\n$usage_msg" >&2
        else
            echo -e "Invalid option -$OPTARG.\n$usage_msg" >&2
        fi
        exit 1
    fi
done

# Shift the positional parameters after processing the options
shift $((OPTIND-1))

# check the value of the etc variable
if [ "$etcs" -eq 1 ]
then
    bin/SOProj ETCS"$etcs" MAPPA"$mappa" # Run the main executable with ETCS1 and MAPPA1
elif [ "$etcs" -eq 2 ]
then
    bin/SOProj ETCS"$etcs" MAPPA"$mappa" RBC &
    bin/SOProj ETCS"$etcs" MAPPA"$mappa" $!  # Run the main executable with ETCS2 and MAPPA1 in the background and run the main executable with ETCS2, MAPPA1, and RBC in the background
else
    echo "ETCS$etcs invalid option" # Print an error message if the value of etcs is invalid
    exit 1
fi