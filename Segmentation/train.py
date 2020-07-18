import datetime
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import backend as K
from tensorflow.keras.layers import Activation, Dense, Input, Conv2D, Flatten, Reshape, Conv2DTranspose
from tensorflow.keras.models import Model, load_model
from tensorflow.keras.datasets import mnist
from tensorflow.keras.preprocessing.image import ImageDataGenerator
from tensorflow.keras.callbacks import TensorBoard
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.image import imsave
from PIL import Image

from Dataset import Dataset
from Loss import dice_coef, dice_coef_loss, binary_dice_coef
from TensorBoardImageCallback import TensorBoardImageCallback

# Setup tensorboard
log_dir = "logs/fit/" + datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
file_writer = tf.summary.create_file_writer(log_dir + "/images")
tensorboard_callback = TensorBoard(log_dir=log_dir, histogram_freq=1)

def generate_sample_weight(size, top_weight):
    sample_weight = np.ones(size)
    for i in range(5, len(sample_weight), 6):
        sample_weight[i] = top_weight
    return sample_weight

def save_predictions(model, dataset, batch_size):
    x_decoded = model.predict(dataset.x, batch_size=batch_size, verbose=1)
    # Prepare image for saving
    x_decoded = np.squeeze(x_decoded, axis=3)
    # Save each image
    for i in range(dataset.x.shape[0]):
        output_image = x_decoded[i]
        output_image_thresholded = (x_decoded[i] >= 0.5) * 1.0
        imsave("output/%d.png" % i, output_image, cmap="Greys_r", vmin=0.0, vmax=1.0)
        imsave("output/%d_threshold.png" % i, output_image_thresholded, cmap="Greys_r", vmin=0.0, vmax=1.0)

# Learning parameters
img_rows = 2056
img_cols = 2454
dataset_train_x_path = 'dataset/train_images'
dataset_train_y_path = 'dataset/train_segmentation'
dataset_validation_x_path = 'dataset/validation_images'
dataset_validation_y_path = 'dataset/validation_segmentation'
data_augmentation = True
train_model = True
model_file = 'sorghum_segmentation_model.h5'
# Network parameters
input_shape = (img_rows, img_cols, 3)
output_shape = (img_rows, img_cols, 1)
relative_top_image_weight = 5.0
batch_size = 4
first_kernel_size = 11
middle_kernel_size = 1
last_kernel_size = 7
epochs = 1000

# Load datasets
dataset_train = Dataset(dataset_train_x_path, dataset_train_y_path, img_rows, img_cols, False)
dataset_train.load()
dataset_validation = Dataset(dataset_validation_x_path, dataset_validation_y_path, img_rows, img_cols, False)
dataset_validation.load()

# Top images have 5 times more weight than side images
sample_weight = generate_sample_weight(dataset_train.x.shape[0], relative_top_image_weight)

if train_model:
    # Build the Autoencoder Model
    # First build the Encoder Model
    input_img = Input(shape=input_shape, name='encoder_input')

    # Simplest model
    x = Conv2D(filters=32, kernel_size=first_kernel_size, activation='relu', padding='same')(input_img)
    x = Conv2D(filters=8, kernel_size=middle_kernel_size, activation='relu', padding='same')(x)
    x = Conv2D(filters=4, kernel_size=middle_kernel_size, activation='relu', padding='same')(x)
    output_img = Conv2D(filters=1, kernel_size=last_kernel_size, activation='sigmoid', padding='same')(x)

    # Instantiate Autoencoder Model
    autoencoder = Model(input_img, output_img, name='autoencoder')
    autoencoder.summary()
    autoencoder.compile(loss=dice_coef_loss, optimizer='adam', metrics=[dice_coef, binary_dice_coef])

    # Callback that saves predictions at every epochs
    tensorboard_image_callback = TensorBoardImageCallback(dataset_validation.x, epochs, file_writer)

    if data_augmentation:
        # Data augmentation
        data_gen_args = dict(rotation_range=90,
                             width_shift_range=0.1,
                             height_shift_range=0.1,
                             shear_range=5,
                             zoom_range=0.1,
                             fill_mode='nearest',
                             horizontal_flip=True,
                             vertical_flip=True)
        image_datagen = ImageDataGenerator(**data_gen_args)
        segmentation_datagen = ImageDataGenerator(**data_gen_args)

        # Provide the same seed and keyword arguments to the fit and flow methods
        seed = 1
        image_datagen.fit(dataset_train.x, augment=True, seed=seed)
        segmentation_datagen.fit(dataset_train.y, augment=True, seed=seed)

        # Combine generators into one which yields image and masks
        # Use option save_to_dir='img_flow' to save augmented images
        train_generator = zip(
            image_datagen.flow(dataset_train.x, batch_size=batch_size, seed=seed, sample_weight=sample_weight),
            segmentation_datagen.flow(dataset_train.y, batch_size=batch_size, seed=seed, sample_weight=sample_weight)
        )

        # Fit the model with a generator
        autoencoder.fit(
            x=train_generator,
            steps_per_epoch=len(dataset_train.x) / batch_size,
            epochs=epochs,
            validation_data=(dataset_validation.x, dataset_validation.y),
            callbacks=[tensorboard_callback, tensorboard_image_callback]
        )

    else:
        # No data augmentation (and no sample_weight, because not supported by Tensorflow 2.1)
        autoencoder.fit(
            x=dataset_train.x,
            y=dataset_train.y,
            validation_data=(dataset_validation.x, dataset_validation.y),
            epochs=epochs,
            batch_size=batch_size,
            callbacks=[tensorboard_callback, tensorboard_image_callback])
    
    # Save the model
    autoencoder.save(model_file)

else:
    # Just load the model without training
    autoencoder = load_model(model_file, custom_objects={
        'dice_coef_loss': dice_coef_loss,
        'dice_coef': dice_coef,
        'binary_dice_coef': binary_dice_coef})
    autoencoder.summary()

# Save predictions in the output folder
save_predictions(autoencoder, dataset_validation, batch_size)
