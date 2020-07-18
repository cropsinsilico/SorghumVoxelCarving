import tensorflow as tf
from tensorflow import keras

class TensorBoardImageCallback(keras.callbacks.Callback):
    def __init__(self, validation_data, last_epoch, file_writer):
        super().__init__()
        # The next epoch to log (1, 2, 4, 8, 16, 32...)
        self.next_epoch_to_log = 1
        self.last_epoch = last_epoch
        self.batch_size = 4
        self.validation_data = validation_data
        self.file_writer = file_writer

    def on_epoch_end(self, epoch, logs={}):
        if epoch >= self.next_epoch_to_log:
            # The next epoch to log is updated (so that we don't log too much information)
            self.next_epoch_to_log = 2 * self.next_epoch_to_log
            # Predict validation data
            data_decoded = self.model.predict(self.validation_data, batch_size=self.batch_size, verbose=0)
            # Export to Tensorboard
            with self.file_writer.as_default():
                tf.summary.image("Validation images", data_decoded, max_outputs=6, step=epoch)

    def on_train_end(self, logs={}):
        # Predict validation data
        data_decoded = self.model.predict(self.validation_data, batch_size=self.batch_size, verbose=0)
        # Export to Tensorboard
        with self.file_writer.as_default():
            tf.summary.image("Validation images", data_decoded, max_outputs=6, step=self.last_epoch + 1)
