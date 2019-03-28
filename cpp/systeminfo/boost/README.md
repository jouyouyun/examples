# SystemInfo

Contains:

- MachineID
- OS
- CPU
- Memory Capacity
- Disk Capacity
- DMI
- Network Device

## Dependencies

- libboost-dev
- libboost-system-dev
- libboost-filesystem-dev
- libcrypto++-dev

## Build

`g++ -Wall -g -c systeminfo.cpp dmi.cpp lsblk.cpp network_device.cpp file.cpp os.cpp -lboost_system -lboost_filesystem -lboost_thread -lpthread -lcrypto++ -I../../`
