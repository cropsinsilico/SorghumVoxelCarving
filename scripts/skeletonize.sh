#!/bin/bash

# File containing the list of plant folders to process
input="plants.txt" 
[ ! -f "$input" ] && { echo "$0 - File $input not found."; exit 1; }

while IFS= read -r folder; do
    # Concatenate the name of the plant folder and the path to the dataset
    calibrationDir="$1/$folder"
    reconstructionDir="$2/$folder"
    outputDir="$3/$folder"
    # If it is indeed a folder
    if test -d "$reconstructionDir"; then
        echo "Processing $reconstructionDir"

        mkdir $outputDir

        ./program/criticalKernelsThinning3D --input "$reconstructionDir/voxels.txt" --select dmax --skel 1isthmus --persistence 1 --verbose --exportTXT "$outputDir/skeleton.txt"
        cp "$reconstructionDir/voxels.txt" "$outputDir/voxels.txt"
        ./program/SorghumReconstruction.exe -c process_skeleton -i $outputDir -o $outputDir

        [ -f "$calibrationDir/0_0_0.png" ] && composite -blend 60  -gravity center "$outputDir/optim_skeleton_0.png" "$calibrationDir/0_0_0.png" "$outputDir/optim_skeleton_0.png" &
        [ -f "$calibrationDir/0_36_0.png" ] && composite -blend 60  -gravity center "$outputDir/optim_skeleton_36.png" "$calibrationDir/0_36_0.png" "$outputDir/optim_skeleton_36.png" &
        [ -f "$calibrationDir/0_72_0.png" ] && composite -blend 60  -gravity center "$outputDir/optim_skeleton_72.png" "$calibrationDir/0_72_0.png" "$outputDir/optim_skeleton_72.png" &
        [ -f "$calibrationDir/0_108_0.png" ] && composite -blend 60  -gravity center "$outputDir/optim_skeleton_108.png" "$calibrationDir/0_108_0.png" "$outputDir/optim_skeleton_108.png" &
        [ -f "$calibrationDir/0_144_0.png" ] && composite -blend 60  -gravity center "$outputDir/optim_skeleton_144.png" "$calibrationDir/0_144_0.png" "$outputDir/optim_skeleton_144.png" &
        [ -f "$calibrationDir/0_216_0.png" ] && composite -blend 60  -gravity center "$outputDir/optim_skeleton_216.png" "$calibrationDir/0_216_0.png" "$outputDir/optim_skeleton_216.png" &
        [ -f "$calibrationDir/0_288_0.png" ] && composite -blend 60  -gravity center "$outputDir/optim_skeleton_288.png" "$calibrationDir/0_288_0.png" "$outputDir/optim_skeleton_288.png" &
        [ -f "$calibrationDir/top_0_90_0.png" ] && composite -blend 60  -gravity center "$outputDir/optim_skeleton_top.png" "$calibrationDir/top_0_90_0.png" "$outputDir/optim_skeleton_top.png" &

        [ -f "$calibrationDir/0_0_0.png" ] && composite -blend 60  -gravity center "$outputDir/raw_skeleton_0.png" "$calibrationDir/0_0_0.png" "$outputDir/raw_skeleton_0.png" &
        [ -f "$calibrationDir/0_36_0.png" ] && composite -blend 60  -gravity center "$outputDir/raw_skeleton_36.png" "$calibrationDir/0_36_0.png" "$outputDir/raw_skeleton_36.png" &
        [ -f "$calibrationDir/0_72_0.png" ] && composite -blend 60  -gravity center "$outputDir/raw_skeleton_72.png" "$calibrationDir/0_72_0.png" "$outputDir/raw_skeleton_72.png" &
        [ -f "$calibrationDir/0_108_0.png" ] && composite -blend 60  -gravity center "$outputDir/raw_skeleton_108.png" "$calibrationDir/0_108_0.png" "$outputDir/raw_skeleton_108.png" &
        [ -f "$calibrationDir/0_144_0.png" ] && composite -blend 60  -gravity center "$outputDir/raw_skeleton_144.png" "$calibrationDir/0_144_0.png" "$outputDir/raw_skeleton_144.png" &
        [ -f "$calibrationDir/0_216_0.png" ] && composite -blend 60  -gravity center "$outputDir/raw_skeleton_216.png" "$calibrationDir/0_216_0.png" "$outputDir/raw_skeleton_216.png" &
        [ -f "$calibrationDir/0_288_0.png" ] && composite -blend 60  -gravity center "$outputDir/raw_skeleton_288.png" "$calibrationDir/0_288_0.png" "$outputDir/raw_skeleton_288.png" &
        [ -f "$calibrationDir/top_0_90_0.png" ] && composite -blend 60  -gravity center "$outputDir/raw_skeleton_top.png" "$calibrationDir/top_0_90_0.png" "$outputDir/raw_skeleton_top.png" &
        
        wait
    fi
done < "$input"
