#!/bin/bash

# Create folders for images
mkdir calibrated
mkdir reconstructed
mkdir segmented
mkdir skeletons

# Create a folder for binary programs
mkdir program
cd program

# Copy the main application
cp ../../x64/Release/SorghumReconstruction.exe SorghumReconstruction.exe
mkdir Images
cp -r ../../SorghumReconstruction/Images/calibration Images
echo "Warning: run windeployqt.exe on this repository"

# Build the skeletonization program
cmake -DCMAKE_BUILD_TYPE=Release ../../Skeletonization
make
