#!/bin/bash

# Strong scalability test

# This script automates the execution of MPI-WordsCount on a fixed input with an increasing number of processors from 1
# up to the value passed as --maxnp argument.

EXECUTABLE="MPI-WordsCount"
REPORTS="reports/"

if [[ $# -lt 7 ]]
then
    echo "Usages:"
    echo -e "\t$0 $EXECUTABLE --maxnp <value> --hostfile <hostfile> -f <filenames>"
    echo -e "\t$0 $EXECUTABLE --maxnp <value> --hostfile <hostfile> -d <dirname>"
    echo -e "\t$0 $EXECUTABLE --maxnp <value> --hostfile <hostfile> -mf <masterfile>"
fi

if [[ $2 = "--maxnp" ]] && [[ $4 = "--hostfile" ]]
then
    MAXNP=$3
    HOSTFILE=$5

    if [[ $6 = "-f" ]]
    then
        shift 6

        for (( i=1; i<=$MAXNP; i++ ))
        do
            echo "Testing with $i processes ... "
            mpirun -np "$i" --hostfile "$HOSTFILE" "$EXECUTABLE" -f "${@}" | tail -2
            echo
        done
    elif [[ $6 = "-d" ]] || [[ $6 = "-mf" ]]
    then
        for (( i=1; i<=$MAXNP; i++ ))
        do
            echo "Testing with $i processes ... "
            mpirun -np "$i" --hostfile "$HOSTFILE" "$EXECUTABLE" "$6" "$7" | tail -2
            echo
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
