#!/bin/bash

debFile=$1
signELF=$2
pkgName=$3
debName=`basename ${debFile}`
outDir=`mktemp /tmp/sign.XXXXXX`
dataDir="data"
hashDir="/usr/share/uos-signverify/signature"

function msg_error() {
    echo "[ERR] $@"
    exit -1
}

rm -f ${outDir}
mkdir -p ${outDir} || msg_error "Failed to mkdir temp dir"
cd ${outDir}
fakeroot dpkg-deb -R ${debFile} ${dataDir} || msg_error "Failed to extract deb file"
mkdir -p ${dataDir}${hashDir} || msg_error "Failed to mkdir signature dir"
${signELF} -action sign -dir ${dataDir} -pkg ${pkgName} -output ${dataDir}${hashDir} || msg_error "Failed to sign"
fakeroot dpkg-deb -b ${dataDir} || msg_error "Failed to build deb file"
mv ${dataDir}.deb ${debName}
