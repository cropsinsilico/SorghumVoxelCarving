import numpy as np

from Images import Images

class Dataset(object):
    """
    Manage the dataset:
        load, preprocess, split train and test set
    """

    def __init__(self, input_path, segmented_path, img_rows, img_cols, shuffle):
        # Target size of images
        self.target_size=(img_rows, img_cols)
        # If true, shuffle the dataset
        self.shuffle = shuffle
        # Path to input training images
        self.input_path = input_path
        # Path to segmented training images
        self.segmented_path = segmented_path
        # Image files
        self.x_files = []
        self.y_files = []
        # Training data set
        self.x = []
        self.y = []


    def load(self):
        (self.x_files, self.x) = self.load_folder(self.input_path)
        (self.y_files, self.y) = self.load_folder(self.segmented_path)
        # Convert from 3 channels to 1 channel
        y_mean_pixel = np.mean(self.y, axis=3)
        # Black background and white plant
        y_mean_pixel = 1.0 - y_mean_pixel
        self.y = np.expand_dims(y_mean_pixel, axis=3)


    def load_folder(self, directory_path):
        # Load real images
        files = Images.list_images_directory(directory_path)
        images = Images.read_list(directory_path, files, self.target_size)
        # Preprocessing
        images = Images.preprocess(images)
        # Shuffle the dataset
        if self.shuffle:
            indices = np.arange(images.shape[0])
            np.random.shuffle(indices)
            files = files[indices]
            images = images[indices]

        return (files, images)


    def print_information(self):
        # Print information on the dataset
        print('Dataset train X shape:', self.x.shape)
        print('Dataset train Y shape:', self.y.shape)
        print(self.x.shape[0], 'x samples')
        print(self.y.shape[0], 'y samples')
        print('No test samples')
