obj-m := file_operations.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement -O3
cwd := $(shell pwd)

kdir := /lib/modules/$(shell uname -r)/build

all:
	make -C ${kdir} M=${cwd} modules

clean:
	make -C ${kdir} M=${cwd} clean
