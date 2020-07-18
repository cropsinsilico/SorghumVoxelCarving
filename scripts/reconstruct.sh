#!/bin/bash

# File containing the list of plant folders to process
input="plants.txt" 
[ ! -f "$input" ] && { echo "$0 - File $input not found."; exit 1; }

while IFS= read -r folder; do
    # Concatenate the name of the plant folder and the path to the dataset
    dir="$2/$folder"
    outputDir="$3/$folder"
    calibrationDir="$1/$folder"
    # If it is indeed a folder
    if test -d "$dir"; then
        echo "Processing $dir"

        mkdir $outputDir

        ./program/SorghumReconstruction.exe -c reconstruction -i $dir -o $outputDir

        [ -f "$calibrationDir/0_0_0.png" ] && composite -blend 30  -gravity center "$outputDir/reprojection_0.png" "$calibrationDir/0_0_0.png" "$outputDir/blend_reprojection_0.png" &
        [ -f "$calibrationDir/0_36_0.png" ] && composite -blend 30  -gravity center "$outputDir/reprojection_36.png" "$calibrationDir/0_36_0.png" "$outputDir/blend_reprojection_36.png" &
        [ -f "$calibrationDir/0_72_0.png" ] && composite -blend 30  -gravity center "$outputDir/reprojection_72.png" "$calibrationDir/0_72_0.png" "$outputDir/blend_reprojection_72.png" &
        [ -f "$calibrationDir/0_108_0.png" ] && composite -blend 30  -gravity center "$outputDir/reprojection_108.png" "$calibrationDir/0_108_0.png" "$outputDir/blend_reprojection_108.png" &
        [ -f "$calibrationDir/0_144_0.png" ] && composite -blend 30  -gravity center "$outputDir/reprojection_144.png" "$calibrationDir/0_144_0.png" "$outputDir/blend_reprojection_144.png" &
        [ -f "$calibrationDir/0_216_0.png" ] && composite -blend 30  -gravity center "$outputDir/reprojection_216.png" "$calibrationDir/0_216_0.png" "$outputDir/blend_reprojection_216.png" &
        [ -f "$calibrationDir/0_288_0.png" ] && composite -blend 30  -gravity center "$outputDir/reprojection_288.png" "$calibrationDir/0_288_0.png" "$outputDir/blend_reprojection_288.png" &
        [ -f "$calibrationDir/top_0_90_0.png" ] && composite -blend 30  -gravity center "$outputDir/reprojection_top.png" "$calibrationDir/top_0_90_0.png" "$outputDir/blend_reprojection_top.png" &

        wait
    fi
done < "$input"
