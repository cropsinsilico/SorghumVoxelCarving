#!/bin/bash

# File containing the list of plant folders to process
input="plants.txt" 
[ ! -f "$input" ] && { echo "$0 - File $input not found."; exit 1; }

while IFS= read -r folder; do
    # Concatenate the name of the plant folder and the path to the dataset
    dir="$1/$folder"
    outputDir="$2/$folder"
    # If it is indeed a folder
    if test -d "$dir"; then
        echo "Processing $dir"

        mkdir $outputDir

        cd program
        [ -f "../$dir/0_0_0.png" ] && ./SorghumReconstruction.exe -c calibration -i "../$dir/0_0_0.png" -o "../$outputDir/0_0_0.png"
        [ -f "../$dir/0_36_0.png" ] && ./SorghumReconstruction.exe -c calibration -i "../$dir/0_36_0.png" -o "../$outputDir/0_36_0.png"
        [ -f "../$dir/0_72_0.png" ] && ./SorghumReconstruction.exe -c calibration -i "../$dir/0_72_0.png" -o "../$outputDir/0_72_0.png"
        [ -f "../$dir/0_108_0.png" ] && ./SorghumReconstruction.exe -c calibration -i "../$dir/0_108_0.png" -o "../$outputDir/0_108_0.png"
        [ -f "../$dir/0_144_0.png" ] && ./SorghumReconstruction.exe -c calibration -i "../$dir/0_144_0.png" -o "../$outputDir/0_144_0.png"
        [ -f "../$dir/0_180_0.png" ] && ./SorghumReconstruction.exe -c calibration -i "../$dir/0_180_0.png" -o "../$outputDir/0_180_0.png"
        [ -f "../$dir/0_216_0.png" ] && ./SorghumReconstruction.exe -c calibration -i "../$dir/0_216_0.png" -o "../$outputDir/0_216_0.png"
        [ -f "../$dir/0_252_0.png" ] && ./SorghumReconstruction.exe -c calibration -i "../$dir/0_252_0.png" -o "../$outputDir/0_252_0.png"
        [ -f "../$dir/0_288_0.png" ] && ./SorghumReconstruction.exe -c calibration -i "../$dir/0_288_0.png" -o "../$outputDir/0_288_0.png"
        [ -f "../$dir/0_324_0.png" ] && ./SorghumReconstruction.exe -c calibration -i "../$dir/0_324_0.png" -o "../$outputDir/0_324_0.png"
        [ -f "../$dir/top_0_90_0.png" ] && ./SorghumReconstruction.exe -c calibration_top -i "../$dir/top_0_90_0.png" -o "../$outputDir/top_0_90_0.png"
        cd ..
    fi
done < "$input"
