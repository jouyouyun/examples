#!/bin/bash

LOGFILE=/tmp/udev-fix-greeter-display.log

function has_monitor_connected() {
    list=`cat /sys/class/drm/*/status`
    for status in ${list[*]}
    do
        if [ $status == "connected" ];then
            return 0
        fi
    done
    return 1
}

function is_runlevel5() {
    list=`systemctl get-default`
    for level in ${list[*]}
    do
        echo "runlevel: ${level}" >> ${LOGFILE}
        if [[ "$level" == "graphical.target" ]] || [[ "$level" == "runlevel5.target" ]]; then
            return 1
        fi
    done
    return 0
}

function has_program_running() {
    echo "Will check program: $1" >> ${LOGFILE}
    pid=`pidof $1`
    ret=$?
    if [ $ret == 0 ]; then
        echo "Has '$1' running" >> ${LOGFILE}
        return 1
    fi

    echo "No '$1' running" >> ${LOGFILE}
    return 0
}

echo "Display monitor changed, start check" > ${LOGFILE}

# check runlevel
is_runlevel5
level5=$?
if [ ${level5} == 0 ]; then
    exit 0
fi

has_monitor_connected
connected=$?
if [ ${connected} == 0 ]; then
    echo "Monitor connected" >> ${LOGFILE}

    programs=("startdde" "lightdm-deepin-greeter" "deepin-installer")
    for prog in ${programs[@]}
    do
        has_program_running ${prog}
        ret=$?
        if [ $ret == 1 ]; then
            exit 0
        fi
    done

    echo "Restart lightdm" >> ${LOGFILE}
    systemctl restart lightdm
else
    echo "No monitor connected" > ${LOGFILE}
fi
