#!/bin/bash

# File containing the list of plant folders to process
input="plants.txt" 
[ ! -f "$input" ] && { echo "$0 - File $input not found."; exit 1; }

segmentationDir="$1"
reconstructionDir="$2"

# Reset output files
cat /dev/null > errors.txt
cat /dev/null > voxels.txt
cat /dev/null > pixels.txt
cat /dev/null > cylinders.txt
cat /dev/null > directionality.txt
cat /dev/null > surface.txt
cat /dev/null > height.txt

while IFS= read -r folder; do
    # Concatenate the name of the plant folder and the path to the dataset
    segmentationFolder="$segmentationDir/$folder"
    reconstructionFolder="$reconstructionDir/$folder"

    # Reprojection error
    if test -d "$reconstructionFolder"; then
        cat "$reconstructionFolder/error.txt" >> errors.txt
    fi

    # Voxel count
    if test -d "$reconstructionFolder"; then
        wc -l "$reconstructionFolder/voxel_centers.obj" >> voxels.txt
    fi

    # Number of pixels in segmented views
    if test -d "$segmentationFolder"; then
        [ -f "$segmentationFolder/0_0_0.png" ] && currentPixels=$(convert "$segmentationFolder/0_0_0.png" -format "%[fx:int((1-mean)*w*h)]" info:) && printf '%s\t' "$currentPixels" >> pixels.txt
        [ -f "$segmentationFolder/0_36_0.png" ] && currentPixels=$(convert "$segmentationFolder/0_36_0.png" -format "%[fx:int((1-mean)*w*h)]" info:) && printf '%s\t' "$currentPixels" >> pixels.txt
        [ -f "$segmentationFolder/0_72_0.png" ] && currentPixels=$(convert "$segmentationFolder/0_72_0.png" -format "%[fx:int((1-mean)*w*h)]" info:) && printf '%s\t' "$currentPixels" >> pixels.txt
        [ -f "$segmentationFolder/0_108_0.png" ] && currentPixels=$(convert "$segmentationFolder/0_108_0.png" -format "%[fx:int((1-mean)*w*h)]" info:) && printf '%s\t' "$currentPixels" >> pixels.txt
        [ -f "$segmentationFolder/0_144_0.png" ] && currentPixels=$(convert "$segmentationFolder/0_144_0.png" -format "%[fx:int((1-mean)*w*h)]" info:) && printf '%s\t' "$currentPixels" >> pixels.txt
        [ -f "$segmentationFolder/0_216_0.png" ] && currentPixels=$(convert "$segmentationFolder/0_216_0.png" -format "%[fx:int((1-mean)*w*h)]" info:) && printf '%s\t' "$currentPixels" >> pixels.txt
        [ -f "$segmentationFolder/0_288_0.png" ] && currentPixels=$(convert "$segmentationFolder/0_288_0.png" -format "%[fx:int((1-mean)*w*h)]" info:) && printf '%s\t' "$currentPixels" >> pixels.txt
        [ -f "$segmentationFolder/top_0_90_0.png" ] && currentPixels=$(convert "$segmentationFolder/top_0_90_0.png" -format "%[fx:int((1-mean)*w*h)]" info:) && printf '%s\t' "$currentPixels" >> pixels.txt
        printf '\n' >> pixels.txt
    fi

    # Density, directionality
    if test -d "$reconstructionFolder"; then
        ./program/SorghumReconstruction.exe -c density -i "$reconstructionFolder/voxels.txt" -o noOutputDir 2>> cylinders.txt
        ./program/SorghumReconstruction.exe -c directionality -i "$reconstructionFolder/voxels.txt" -o noOutputDir 2>> directionality.txt
        ./program/SorghumReconstruction.exe -c surface -i "$reconstructionFolder/voxels.txt" -o noOutputDir 2>> surface.txt
        ./program/SorghumReconstruction.exe -c height -i "$reconstructionFolder/voxels.txt" -o noOutputDir 2>> height.txt
    fi

done < "$input"
