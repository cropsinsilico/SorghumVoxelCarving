## Setup
- Main program 
    - Open the Visual Studio 2019 solution in `SorghumReconstruction.sln`
    - Build in Release mode

Run `setup.py` to build the skeletonization program and move binary files to the correct folder
```bash
$ bash setup.py
```

Don't forget to run `windeployqt.exe` on the program folder, to copy all the necessary DLLs.

Add the list of plants to reconstruct in the a file named `plants.txt` (make sure you use Unix line return).
Each line of this file is a folder in the `dataset` directory.
```txt
4-9-18_Schnable_49-387-js261-419_2018-04-11_04-03-51_9979400
```

## Run the entire reconstruction
Execute the following commands, in this order:

```bash
$ bash calibrate.sh dataset calibrated
$ bash segment.sh calibrated segmented
$ bash reconstruct.sh calibrated segmented reconstructed
$ bash measure.sh segmented reconstructed
$ ./program/SorghumReconstruction.exe -c train_classifier -i training -o model.yml
$ bash skeletonize.sh calibrated reconstructed skeletons

# Optional
$ bash dumpPlant.sh dataset calibrated segmented reconstructed skeletons 4-9-18_Schnable_49-387-js261-419_2018-04-11_04-03-51_9979400 output
```

## Result
Dataset
-------
Each subfolder of the directory contains:
- dataset: original images of the plants
- calibration: calibrated images with a mask to help segmentation
- segmentation: segmented images
- reconstruction/voxel_centers.obj: OBJ file containing the center of voxels as vertices 
- reconstruction/voxels.txt: TXT file with the indices of voxels in a grid of resolution 512
- reconstruction/error.txt: Value of the Dice coefficient between reprojected voxels and segmented views
- reconstruction/reprojection_*.png: Map of the Dice coefficient in reprojections 
- reconstruction/blend_reprojection_*.png: Reprojections blended with calibrated images
- skeleton/error.txt: Maximum distance from a voxel to the skeleton
- skeleton/topology.txt: True if the stem has no T-junction
- skeleton/skeleton.txt: TXT file with the indices of voxels in the raw skeleton
- skeleton/optim_skeleton.txt: TXT file with the indices of voxels in the final skeleton
- skeleton/optim_skeleton.obj: OBJ file with the voxels of the final skeleton
- skeleton/raw_skeleton_*.png: Reprojections blended with the raw skeleton
- skeleton/optim_skeleton_*.png: Reprojections blended with the final skeleton