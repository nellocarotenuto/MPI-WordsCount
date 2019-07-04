#!/bin/bash

# Weak scalability test

# This script automates the execution of MPI-WordsCount with an increasing number of processors from 1 up to the value
# passed as --maxnp argument each called on the file passed as argument.

EXECUTABLE="MPI-WordsCount"
REPORTS="reports/"
EXECTIMES="performances/weakscalability"

if [[ $# -lt 9 ]]
then
    echo "Usage:"
    echo -e "\t$0 --maxnp <value> --hostfile <hostfile> --runs <runs> $EXECUTABLE -f <filenames>"
    exit
fi

if [[ $1 = "--maxnp" ]] && [[ $3 = "--hostfile" ]] && [[ $5 = "--runs" ]]
then
    mkdir -p "$EXECTIMES"

    MAXNP=$2
    HOSTFILE=$4
    RUNS=$6

    if [[ $MAXNP <= 0 ]] || [[ $RUNS <= 0 ]]
    then
        echo "Arguments --maxnp and --runs must be positive."
        exit
    fi

    DATE="$(date '+%Y-%m-%d %H:%M:%S')"
    FILENAME="$EXECTIMES/$DATE.csv"

    if [[ $8 = "-f" ]]
    then
        shift 8
        FILES=("${@}")

        for (( i=1; i<=$MAXNP; i++ ))
        do
            for (( j=0; j<$i; j++ ))
            do
                ARGUMENTS+=("${FILES[@]}")
            done

            for (( r=1; r<=$RUNS; r++))
            do

                echo "Testing with $i processes ... "
                REPORT="$(mpirun -np "$i" --hostfile "$HOSTFILE" "$EXECUTABLE" -f "${ARGUMENTS[@]}" | tail -2)"

                echo "$REPORT"
                echo

                TIME="$(echo $REPORT | cut -d' ' -f 3 | cut -d's' -f 1)"

                if [[ $r != $RUNS ]]
                then
                    echo -n "$TIME, " >> "$FILENAME"
                else
                    echo "$TIME" >> "$FILENAME"
                fi
            done

            unset ARGUMENTS
        done
    else
        echo "Unknown $EXECUTABLE option \""$8"\""
        exit
    fi

    echo "Tests done! Check reports in $REPORTS folder."
    echo "Execution times are reported in $EXECTIMES folder."
    exit
else
    echo "Unknown options \""$1"\" "$3"\" "$5"\"."
    exit
fi
