import argparse
from os.path import isfile, isdir, join

import numpy as np

from tensorflow.keras.models import load_model

from matplotlib.image import imsave

from Dataset import Dataset
from Images import Images
from Loss import dice_coef, dice_coef_loss, binary_dice_coef

target_size = (2056, 2454)
batch_size = 4

def main():
    # Parse program arguments
    parser = argparse.ArgumentParser()

    parser.add_argument('--input',
                        type=str,
                        help='Path to the image to process')

    parser.add_argument('--output',
                        type=str,
                        help='Path to the output image')

    parser.add_argument('--model',
                        type=str,
                        help='Path to the Keras model file')

    args = parser.parse_args()

    # Load Keras model if present
    if isfile(args.model):
        model = load_model(args.model, custom_objects={
            'dice_coef_loss': dice_coef_loss,
            'dice_coef': dice_coef,
            'binary_dice_coef': binary_dice_coef})
        model.summary()

        # If input is a file and exists
        if isfile(args.input):
            # Load and preprocess input image
            input_image = Images.read(args.input, target_size)
            input_image = Images.preprocess(input_image)

            # Predict input
            predictions = model.predict(input_image, batch_size=batch_size, verbose=1)
            # Save the first and only image in the result
            predictions = np.squeeze(predictions, axis=3)
            output_image = (predictions[0] >= 0.5) * 1.0
            imsave(args.output, output_image, cmap="Greys_r", vmin=0.0, vmax=1.0)

        # If input and output are directories
        elif isdir(args.input) and isdir(args.output):
            # Load all images from the input
            dataset = Dataset(args.input, args.input, target_size[0], target_size[1], False)
            (input_files, input_images) = dataset.load_folder(args.input)

            # Predict input
            predictions = model.predict(input_images, batch_size=batch_size, verbose=1)
            # Save the first and only image in the result
            predictions = np.squeeze(predictions, axis=3)
            # Save each image
            for i in range(input_images.shape[0]):
                output_image = (predictions[i] >= 0.5) * 1.0
                output_filename = join(args.output, input_files[i])
                imsave(output_filename, output_image, cmap="Greys_r", vmin=0.0, vmax=1.0)


if __name__ == "__main__":
    main()