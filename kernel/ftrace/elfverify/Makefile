obj-m += elf_verify.o
elf_verify-objs := utils.o partition.o chdev.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement -Werror  -O3
cwd := $(shell pwd)

kdir := /lib/modules/$(shell uname -r)/build

all:
	make -C ${kdir} M=$(cwd) modules

clean:
	make -C ${kdir} M=$(cwd) clean
