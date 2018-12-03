#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import json
from legalization import Legalization
from machines import Machines
from overview import Overview

def usage():
    print("""This proccess generate legalization standing book(machines or overview).
    Usage: %s <standing book type: machines or overview> <data file> <output file>
    """ % sys.argv[0])
    sys.exit(-1)

def loadFile(filename):
    try:
        with open(filename, "r") as fr:
            infos = json.load(fr)
    except Exception as e:
        print("Failed to load file:", e)
        return None
    return infos

def main():
    if len(sys.argv) != 4:
        usage()

    infos = loadFile(sys.argv[2])
    if infos == None:
        sys.exit(-1)

    lega = Legalization()
    if sys.argv[1] == "machines":
        lega = Machines(infos)
    elif sys.argv[1] == "overview":
        lega = Overview(infos)

    lega.generate_file(sys.argv[3])
    return

if __name__ == "__main__":
    main()
