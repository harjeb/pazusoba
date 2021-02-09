import os, sys
import cv2
import numpy as np

class Phone:

    def __init__(self, devicename=''):
        self.devicename = devicename


    # calc center point
    def findpic(self, template, origin, value=0.90):
        img = cv2.imread(origin, 0)
        template = cv2.imread(template, 0)
        w, h = template.shape[::-1]
        res = cv2.matchTemplate(img,template,cv2.TM_CCOEFF_NORMED)
        min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(res)

        if max_val > value:
            top_left = max_loc
            bottom_right = (top_left[0] + w, top_left[1] + h)
            #print(top_left, bottom_right)
            #print(max_val)
            loc = (int((bottom_right[0]+top_left[0])/2), int((bottom_right[1]+top_left[1])/2))
            return (loc)
        else:
            print('Not found pic!!!')
            return None

    def exist(self, temp, origin, value=0.8):

        img = cv2.imread(origin, 0)
        template = cv2.imread(temp, 0)
        # print(origin)
        # print(temp)
        res = cv2.matchTemplate(img,template,cv2.TM_CCOEFF_NORMED)
        min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(res)

        if max_val > value:
            print('semilter:' + str(max_val))
            return True
        else:
            return False

