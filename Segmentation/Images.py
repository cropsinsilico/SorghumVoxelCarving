from os import listdir
from os.path import isfile, join

from tensorflow.keras.preprocessing import image

import numpy as np

try:
    from PIL import Image as pil_image
except ImportError:
    pil_image = None

class Images(object):

    @staticmethod
    def _load_image(image_path, target_size):
        """
        Load an image in grayscale
        Reimplement load_img from Keras to support 16 bits grayscale images
        """
        if pil_image is None:
            raise ImportError('Could not import PIL.Image. '
                              'The use of `array_to_img` requires PIL.')
        img = pil_image.open(image_path)

        if img.mode not in ['L', 'I']:
            raise ImportError('The PIL mode of the image is not valid.')

        if target_size is not None:
            width_height_tuple = (target_size[1], target_size[0])
            if img.size != width_height_tuple:
                img = img.resize(width_height_tuple, pil_image.BICUBIC)

        img_array = image.img_to_array(img)
        img_array = np.expand_dims(img_array, axis=0)
        return img_array


    @staticmethod
    def list_images_directory(dir_path):
        """
        List all PNG and TIF images in a directory
        """
        files = []
        for filename in listdir(dir_path):
            path = join(dir_path, filename)
            if isfile(path) and path.lower().endswith(('.png', '.tif')):
                files.append(filename)
        files.sort()
        return files


    @staticmethod
    def preprocess(images):
        """
        Preprocess all images in the dataset
        Center all samples with the mean of the whole dataset
        """
        # Rescale to approximately [0; 1]
        images = images.astype('float32')
        images = images / (255.0)

        mean = np.mean(images)
        std = np.std(images)
        min = np.amin(images)
        max = np.amax(images)
        print('mean: %.2f\tstd: %.2f\tmin: %.2f\tmax: %.2f' % (mean, std, min, max))
        # images = (images - mean)/std

        return images


    @staticmethod
    def reverse_preprocess(images):
        images = images * (255.0)
        return images
    

    @classmethod
    def read(cls, file_path, target_size=None):
        """
        Read a single image
        """
        img = image.load_img(file_path, target_size)
        img_array = image.img_to_array(img)
        return np.expand_dims(img_array, axis=0)


    @classmethod
    def read_list(cls, directory_path, image_files, target_size=None):
        """
        Load all images given in a list in a directory
        """
        images = []
        for img_file in image_files:
            image_path = join(directory_path, img_file)
            img_array = cls.read(image_path, target_size)
            images.append(img_array)
        
        return np.concatenate(images)


    @classmethod
    def read_directory(cls, directory_path, target_size=None):
        """
        Load all images in a directory
        """
        image_list = cls.list_images_directory(directory_path)
        return cls.load_images(directory_path, image_list, target_size)

