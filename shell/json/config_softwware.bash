#!/bin/bash

VALUES=$1

for s in $(echo $VALUES | jq -r "to_entries|map(\"\(.key)=\(.value|tostring)\")|.[]"); do
    export $s
done

function configSoftware() {
    local markSoft=$1
    markSoft=${markSoft//\[/}
    markSoft=${markSoft//\]/}
    local markList=(${markSoft//,/ })

    local availSoft=$2
    availSoft=${availSoft//\[/}
    availSoft=${availSoft//\]/}
    local availList=(${availSoft//,/ })

    declare -a removeList
    idx=0
    for mark in "${markList[@]}"; do
        found=0
        for avail in "${availList[@]}"; do
            if [ $mark == $avail ]; then
                found=1
            fi
        done
        if [ $found == 0 ]; then
            removeList[$idx]=$mark
            ((idx++))
        fi
    done

    removeLen=${#removeList[@]}
    if [ $removeLen != 0 ]; then
        # remove
        echo "Will remove: apt-get remove ${removeList[@]}"
        # apt-get remove --allow-remove-essential ${removeList[@]}
    fi
    availLen=${#availList[@]}
    if [ $availLen != 0 ]; then
        # install
        echo "Will install: apt-get install -y ${availList[@]}"
        # apt-get install -y -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold"  ${availList[@]}
    fi
}

# MARK_SOFTWARES=`echo "$1" | jq -c '.MARK_SOFTWARES'`
# AVAILABLE_SOFTWARES=`echo "$1" | jq -c '.AVAILABLE_SOFTWARES'`
echo "Mark: ${MARK_SOFTWARES}"
echo "Avail: ${AVAILABLE_SOFTWARES}"

if [ ! -z ${MARK_SOFTWARES} ]; then
    echo "Has mark"
fi

if [ ! -z ${AVAILABLE_SOFTWARES} ]; then
    echo "Has avail"
fi

 if [ ! -z ${MARK_SOFTWARES} -a ! -z ${AVAILABLE_SOFTWARES} ]; then
	 configSoftware ${MARK_SOFTWARES} ${AVAILABLE_SOFTWARES}
 fi
