#!/bin/bash

# File containing the list of plant folders to process
input="plants.txt" 
[ ! -f "$input" ] && { echo "$0 - File $input not found."; exit 1; }

skeletonDir="$1"

# Reset output files
cat /dev/null > errors.txt
cat /dev/null > topology.txt

while IFS= read -r folder; do
    # Concatenate the name of the plant folder and the path to the dataset
    skeletonFolder="$skeletonDir/$folder"

    # Skeleton error
    if test -f "$skeletonFolder/error.txt"; then
        cat "$skeletonFolder/error.txt" >> errors.txt
    else
        echo "error" >> errors.txt
    fi

    # Topology
    if test -f "$skeletonFolder/topology.txt"; then
        cat "$skeletonFolder/topology.txt" >> topology.txt
    else
        echo "error" >> topology.txt
    fi

done < "$input"
