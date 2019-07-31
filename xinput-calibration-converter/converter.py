#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

import sys

def usage():
    info = "Usage: " + sys.argv[0]
    info += " <screen_width> <screen_height>"
    info += " <click_0_x> <click_0_y>"
    info += " <click_3_x> <click_3_y>"
    print(info)
    print("\tScreen width/hight by the command 'xrandr|grep screen' got")
    print("\tClick x/y by the command 'xinput_calibrator -v' got")
    sys.exit(0)

def convert(screen_x, screen_y, c0_x, c0_y, c3_x, c3_y):
    a = (screen_x * 6 / 8) / (c3_x - c0_x)
    c = ((screen_x / 8) - (a * c0_x)) / screen_x
    e = (screen_y * 6 / 8) / (c3_y - c0_y)
    f = ((screen_y / 8) - (e * c0_y)) / screen_y

    print("Try set 'libinput Calibration Matrix' to '%.1f, 0.0, %.1f, 0.0, %.1f, %.1f, 0.0, 0.0, 1.0'" % (a,c,e, f))

if __name__ == "__main__":
    if len(sys.argv) != 7:
        usage()
    
    convert(int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]), 
        int(sys.argv[4]),int(sys.argv[5]),int(sys.argv[6]))
