#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json

def loadFile(filename):
    try:
        with open(filename, "r") as fr:
            dataObj = json.load(fr)
    except Exception as e:
        print("Failed to load file:", e)
        return None
    return dataObj

def dumpLastoreConfig(filename):
    dataObj = loadFile(filename)
    if dataObj == None:
        return

    print("Lastore config:")
    keys = ["AutoCheckUpdates", "AutoDownloadUpdates", "AutoClean", "MirrorSource"]
    print("\t%s: %s" % (keys[0], dataObj[keys[0]]))
    print("\t%s: %s" % (keys[1], dataObj[keys[1]]))
    print("\t%s: %s" % (keys[2], dataObj[keys[2]]))
    print("\t%s: %s" % (keys[3], dataObj[keys[3]]))

if __name__ == "__main__":
    dumpLastoreConfig("/var/lib/lastore/config.json")
