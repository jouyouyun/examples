#!/bin/bash

ENTRIES=`cat /proc/acpi/wakeup|grep enabled|awk '{print $1}'|xargs`
for i in ${ENTRIES}; do
	echo "Entry: $i"
	if [ $i != "PBTN"  ]; then
		echo "echo $i|tee /proc/acpi/wakeup"
		echo $i|tee /proc/acpi/wakeup
	fi
done
