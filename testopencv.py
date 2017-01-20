from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2
import math
import numpy as np

##--------------------------------------------------------------------------------
# Color Balance Methods 

def apply_mask(matrix, mask, fill_value):
    masked = np.ma.array(matrix, mask=mask, fill_value=fill_value)
    return masked.filled()
 
def apply_threshold(matrix, low_value, high_value):
    low_mask = matrix < low_value
    matrix = apply_mask(matrix, low_mask, low_value)
 
    high_mask = matrix > high_value
    matrix = apply_mask(matrix, high_mask, high_value)
 
    return matrix

def simplest_cb(img, percent):
    assert img.shape[2] == 3
    assert percent > 0 and percent < 100
 
    half_percent = percent / 200.0
 
    channels = cv2.split(img)
 
    out_channels = []
    for channel in channels:
        assert len(channel.shape) == 2
        # find the low and high precentile values (based on the input percentile)
        height, width = channel.shape
        vec_size = width * height
        flat = channel.reshape(vec_size)
 
        assert len(flat.shape) == 1
 
        flat = np.sort(flat)
 
        n_cols = flat.shape[0]
 
        low_val  = flat[math.floor(n_cols * half_percent)]
        high_val = flat[math.ceil( n_cols * (1.0 - half_percent))]
 
        # saturate below the low percentile and above the high percentile
        thresholded = apply_threshold(channel, low_val, high_val)
        # scale the channel
        normalized = cv2.normalize(thresholded, thresholded.copy(), 0, 255, cv2.NORM_MINMAX)
        out_channels.append(normalized)
 
    return cv2.merge(out_channels)

##--------------------------------------------------------------------------------
# Main Loop

# Using PiCamera:
camera = PiCamera()
camera.resolution = (640, 480)
camera.framerate = 32
rawCapture = PiRGBArray(camera, size=(640, 480))

# Allow camera to warm up
time.sleep(0.1)

# Initialize variables
frameWidth = 480
frameHeight = 640
zoom = 1
x = math.floor((((frameWidth / zoom) * (zoom / 2.0)) - ((frameWidth / zoom) / 2.0)))
y = math.floor((((frameHeight / zoom) * (zoom / 2.0))- ((frameHeight / zoom) / 2.0)))
width = math.floor((frameWidth / zoom))
height = math.floor((frameHeight / zoom))
sharp = False
colorbalance = False

for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
    # Grab the numpy array
    image = frame.array

    cropped = image[x:x+width, y:y+height]
    res = cv2.resize(cropped, None, fx=zoom, fy=zoom, interpolation = cv2.INTER_LINEAR)
    if sharp:
        kernel = np.array([[-1,-1,-1], [-1,9,-1], [-1,-1,-1]])
        res = cv2.filter2D(res, -1, kernel)

    if colorbalance:
        res = simplest_cb(res, 1)
    
    #cv2.imshow('sharp',res)
    cv2.imshow('Frame', res)

    ##-------------------------------------------------------------------
    # User input portion, for testing 

    
    key = cv2.waitKey(1) & 0xFF

    rawCapture.truncate(0)

    # ESC to Exit Script
    if key == 27:
        break

    # 1 through 8 to set Zoom
    elif key >= 49 and key <= 56:
        # Get the numeric value from 1-8
        zoom = key - 48

        # Calculate the region of interest of the zoom
        x = math.floor((((frameWidth / zoom) * (zoom / 2.0)) - ((frameWidth / zoom) / 2.0)))
        y = math.floor((((frameHeight / zoom) * (zoom / 2.0))- ((frameHeight / zoom) / 2.0)))
        width = math.floor((frameWidth / zoom))
        height = math.floor((frameHeight / zoom))
        print "{}: {},{} {},{}".format(zoom, x, y, width, height)

    # q to set Sharpen on, w to turn Sharpen off
    elif key == 113:
        sharp = True
        print "Sharp On"
    elif key == 119:
        sharp = False
        print "Sharp Off"

    # a to set Color Balance on, s to turn Color Balance off
    elif key == 97:
        colorbalance = True
        print "Color Balance On"
    elif key == 115:
        colorbalance = False
        print "Color Balance Off"

    # Print the key value of any other keypress, to help if we want to assign more.
    elif key != 255:
        print "Key: {}".format(key)
