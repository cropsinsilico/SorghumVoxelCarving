from tensorflow.keras import backend as K

# Dice Loss
# Source: https://github.com/keras-team/keras/issues/3611
# Similar to: https://lars76.github.io/neural-networks/object-detection/losses-for-segmentation/
def dice_coef(y_true, y_pred, smooth=1):
    y_true_f = K.flatten(y_true)
    y_pred_f = K.flatten(y_pred)
    intersection = K.sum(y_true_f * y_pred_f)
    return (2.0 * intersection + smooth) / (K.sum(y_true_f) + K.sum(y_pred_f) + smooth)

def dice_coef_loss(y_true, y_pred):
    return 1.0 - dice_coef(y_true, y_pred)

def binary_dice_coef(y_true, y_pred, smooth=1):
    y_true_f = K.flatten(y_true)
    y_pred_f = K.flatten(y_pred)
    # Thresholding predictions
    y_pred_f = K.cast(K.greater(K.clip(y_pred_f, 0.0, 1.0), 0.5), K.floatx())
    intersection = K.sum(y_true_f * y_pred_f)
    return (2.0 * intersection + smooth) / (K.sum(y_true_f) + K.sum(y_pred_f) + smooth)
    