#!/bin/bash

# Strong scalability test

# This script automates the execution of MPI-WordsCount on a fixed input with an increasing number of processors from 1
# up to the value passed as --maxnp argument.

EXECUTABLE="MPI-WordsCount"
REPORTS="reports/"
EXECTIMES="performances/strongscalability"

if [[ $# -lt 9 ]]
then
    echo "Usages:"
    echo -e "\t$0 --maxnp <value> --hostfile <hostfile> --runs <runs> $EXECUTABLE -f <filenames>"
    echo -e "\t$0 --maxnp <value> --hostfile <hostfile> --runs <runs> $EXECUTABLE -d <dirname>"
    echo -e "\t$0 --maxnp <value> --hostfile <hostfile> --runs <runs> $EXECUTABLE -mf <masterfile>"
fi

if [[ $1 = "--maxnp" ]] && [[ $3 = "--hostfile" ]] && [[ $5 = "--runs" ]]
then
    mkdir -p "$EXECTIMES"

    MAXNP=$2
    HOSTFILE=$4
    RUNS=$6

    if [[ "$MAXNP" -le 0 ]] || [[ "$RUNS" -le 0 ]]
    then
        echo "Arguments --maxnp and --runs must be positive."
        exit
    fi

    DATE="$(date '+%Y-%m-%d %H:%M:%S')"
    FILENAME="$EXECTIMES/$DATE.csv"

    if [[ $8 = "-f" ]] || [[ $8 = "-d" ]] || [[ $8 = "-mf" ]]
    then
        MODE="$8"

        if [[ $8 = "-f" ]]
        then
            shift 8

            ARGUMENTS=("$@")
        elif [[ $8 = "-d" ]] || [[ $8 = "-mf" ]]
        then
            ARGUMENTS="$9"
        fi

        for (( i=1; i<=$MAXNP; i++ ))
        do

            for (( r=1; r<=$RUNS; r++))
            do

                echo "Testing with $i processes ... "
                REPORT="$(mpirun -np "$i" --hostfile "$HOSTFILE" "$EXECUTABLE" "$MODE" "${ARGUMENTS[@]}" | tail -2)"

                echo "$REPORT"
                echo

                TIME="$(echo $REPORT | cut -d' ' -f 3 | cut -d's' -f 1)"

                if [[ "$r" -ne "$RUNS" ]]
                then
                    echo -n "$TIME, " >> "$FILENAME"
                else
                    echo "$TIME" >> "$FILENAME"
                fi
            done
        done
    else
        echo "Unknown $EXECUTABLE option \""$8"\""
        exit
    fi

    echo "Tests done! Check full reports in $REPORTS folder."
    echo "Execution times are reported in \"$FILENAME\"."
    exit
else
    echo "Unknown options \""$1"\" "$3"\" "$5"\"."
    exit
fi
