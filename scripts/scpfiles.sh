#!/bin/sh

# This script automates the process of copying the needed files onto the workers.
#
# You have to manually copy the files you need onto the master node and then launch this script passing the hosts file
# as argument.
#
# This script requires your master node to be already capable of connecting to each worker through SSH without password.

if [ "$#" -lt 2 ]
then
	echo "Usage: $0 --hostfile <filename> [--user <username>]"
	exit
fi

EXECUTABLE="MPI-WordsCount"
DATADIR="../data/"

USER="ubuntu"

if [ "$#" -eq 4 ] && [ $3 = "--user" ]
then
	USER=$4
fi

if [ "$1" = "--hostfile" ]
then
	while IFS=' ' read -r ADDRESS slots
	do
		if [ "$ADDRESS" != "localhost" ] && [ "$ADDRESS" != "127.0.0.1" ]
		then
			echo "Copying files to $ADDRESS ..."
			scp -r "$EXECUTABLE" "$DATADIR" "$USER"@"$ADDRESS":"$PWD"
			echo
		fi
	done < "$2"

    echo "Files copied onto the slaves!"
    exit
else
    echo "Unknown option \""$1"\""
    exit
fi
