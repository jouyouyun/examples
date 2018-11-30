#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import json
import legalization

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

if __name__ == "__main__":
    if len(sys.argv) != 4:
        usage()

    infos = loadFile(sys.argv[2])
    if infos == None:
        sys.exit(-1)

    if sys.argv[1] == "machines":
        legalization.genFile(infos, sys.argv[3])
    elif sys.argv[1] == "overview":
        pass
