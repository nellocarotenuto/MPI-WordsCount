#!/bin/bash

# Weak scalability test

# This script automates the execution of MPI-WordsCount with an increasing number of processors from 1 up to the value
# passed as --maxnp argument each called on the file passed as argument.

EXECUTABLE="MPI-WordsCount"
REPORTS="reports/"

if [[ $# -lt 5 ]]
then
    echo "Usage:"
    echo -e "\t$0 $EXECUTABLE --maxnp <value> -f <filenames>"
    exit
fi

if [[ $2 = "--maxnp" ]]
then
    MAXNP=$3

    if [[ $4 = "-f" ]]
    then
        shift 4
        FILES=("${@}")

        for (( i=1; i<=$MAXNP; i++ ))
        do
            for (( j=0; j<$i; j++ ))
            do
                ARGUMENTS+=("${FILES[@]}")
            done

            echo "Testing with $i processes ... "
            mpirun -np "$i" "$EXECUTABLE" -f "${ARGUMENTS[@]}" | tail -2
            echo

            unset ARGUMENTS
        done
    fi
else
    echo "Unknown option \""$2"\""
    exit
fi

echo "Tests done! Check reports in $REPORTS folder."
exit