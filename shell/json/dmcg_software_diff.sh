#!/bin/bash

if [ "$1"x == ""x ]; then
	echo "Please input mark and available json"
	exit
fi

if [ "$2"x == ""x ]; then
	echo "Please input mark and available json"
	exit
fi

markRet=`cat $1 | jq -c '[.[]]'`
echo "Software Mark: ${markRet}"
markRet=${markRet//\[/}
markRet=${markRet//\]/}
markList=(${markRet//,/ })

availRet=`cat $2 | jq -c '[.available_softwares[]]'`
echo "Available List: ${availRet}"
availRet=${availRet//\[/}
availRet=${availRet//\]/}
availList=(${availRet//,/ })

function is_in_available_list() {
	local name=$1
	local list=$2

	for avail in "${availList[@]}"; do
		echo -e "\tCompare: ${avail}, ${name}"
		if [ "$avail"x == "$name"x ]; then
			return 1
		fi
	done

	return 0
}

declare -a removeList
idx=0
for mark in "${markList[@]}"; do
	echo -e "\nStart: ${mark}"
	is_in_available_list ${mark}
	ret=$?
	echo "Ret: $ret, $idx"
	if [ "${ret}"x == "0"x ]; then
		removeList[$ret]=$mark
		echo "After insert: ${removeList}"
		((idx++))
	fi
done

removeLen=${#removeList[@]}
if [ $removeLen == 0 ]; then
	echo "nothing to do"
	exit
fi
echo "Remove len(${removeLen}): apt-get purge ${removeList[@]}"
