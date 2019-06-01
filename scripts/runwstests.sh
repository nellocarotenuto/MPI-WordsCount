#!/bin/bash

# Weak scalability test

# This script automates the execution of MPI-WordsCount with an increasing number of processors from 1 up to the value
# passed as --maxnp argument each called on the file passed as argument.

EXECUTABLE="MPI-WordsCount"
REPORTS="reports/"

if [[ $# -lt 7 ]]
then
    echo "Usage:"
    echo -e "\t$0 $EXECUTABLE --maxnp <value> --hostfile <hostfile> -f <filenames>"
    exit
fi

if [[ $2 = "--maxnp" ]] && [[ $4 = "--hostfile" ]]
then
    MAXNP=$3
    HOSTFILE=$5

    if [[ $6 = "-f" ]]
    then
        shift 6
        FILES=("${@}")

        for (( i=1; i<=$MAXNP; i++ ))
        do
            for (( j=0; j<$i; j++ ))
            do
                ARGUMENTS+=("${FILES[@]}")
            done

            echo "Testing with $i processes ... "
            mpirun -np "$i" --hostfile "$HOSTFILE" "$EXECUTABLE" -f "${ARGUMENTS[@]}" | tail -2
            echo

            unset ARGUMENTS
        done
    else
        echo "Unknown $EXECUTABLE option \""$6"\""
        exit
    fi
else
    echo "Unknown options \""$2"\" "$4"\"."
    exit
fi

echo "Tests done! Check reports in $REPORTS folder."
exit