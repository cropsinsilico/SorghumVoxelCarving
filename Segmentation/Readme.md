### Install the virtual environment to run segmentation
```bash
$ virtualenv --system-site-packages -p python3.7 tf21ip37
$ source tf21ip37/bin/activate
$ pip install --upgrade pip
$ pip install --upgrade intel-tensorflow Pillow matplotlib
```

### Dataset folder
* dataset/
    * train_images
    * train_segmentation
    * validation_images
    * validation_segmentation

### Running the training in background
```bash
$ nohup python train.py &
```

### Access Tensorboard on the server from SSH
```bash
# Redirect port 6006 from the server to the client
$ ssh -L6006:localhost:6006 128.210.171.155
# Open http://localhost:6006
```
