#!/bin/bash

folder="$6"
datasetDir="$1/$folder"
calibrationDir="$2/$folder"
segmentationDir="$3/$folder"
reconstructionDir="$4/$folder"
skeletonDir="$5/$folder"
outputDir="$7"

mkdir "$outputDir"

# Move the original dataset
mkdir "$outputDir/dataset"
cp -a "$datasetDir/." "$outputDir/dataset"

# Move the calibration images
mkdir "$outputDir/calibration"
cp -a "$calibrationDir/." "$outputDir/calibration"

# Move the segmentation images
mkdir "$outputDir/segmentation"
cp -a "$segmentationDir/." "$outputDir/segmentation"

# Move the reconstruction files
mkdir "$outputDir/reconstruction"
cp -a "$reconstructionDir/." "$outputDir/reconstruction"

# Move the skeleton files
mkdir "$outputDir/skeleton"
cp -a "$skeletonDir/." "$outputDir/skeleton"