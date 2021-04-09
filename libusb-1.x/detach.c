/**
 * Copyright (C) 2021 jouyouyun <jouyouwen717@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * detach.c -- test usb detach by libusb-1.0
 *
 * compile: gcc -Wall -g detach.c `pkg-config --libs --cflags libusb-1.0`
 *
 * Written on 星期三,  7 四月 2021.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

libusb_context *ctx = NULL;

static void detach_device(const char *vendor, const char *product)
{
	int ret = 0;
	long int vid = 0, pid = 0;

	vid = strtol(vendor, NULL, 16);
	pid = strtol(product, NULL, 16);

	printf("will detach device %lx:%lx\n", vid, pid);
	libusb_device_handle *handler = libusb_open_device_with_vid_pid(ctx, vid, pid);
	if (!handler) {
		fprintf(stderr, "failed to open device(%lx:%lx):", vid, pid);
		return;
	}

	ret = libusb_detach_kernel_driver(handler, 0);
	if (ret != LIBUSB_SUCCESS) {
		fprintf(stderr, "failed to detach device(%lx:%lx): %s\n", vid, pid, libusb_strerror(ret));
	}

	libusb_close(handler);
}

// Some USB devices of the same model may have the same vendor id and product id.
static void detach_device2(const char *vendor, const char *product)
{
	int i = 0;
	int ret = 0;
	long int vid = 0, pid = 0;
	struct libusb_device **devs = NULL;
	struct libusb_device *dev = NULL;
	struct libusb_device_handle *handler = NULL;

        vid = strtol(vendor, NULL, 16);
        pid = strtol(product, NULL, 16);

        printf("will detach device %lx:%lx\n", vid, pid);
	if ((ret = libusb_get_device_list(ctx, &devs)) < 0) {
          fprintf(stderr, "failed to get device list: %s\n", libusb_strerror(ret));
          return;
	}

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		ret = libusb_get_device_descriptor(dev, &desc);
		if (ret < 0) {
                  fprintf(stderr, "failed to get device descriptor: %s\n", libusb_strerror(ret));
		  goto out;
                }

		if (desc.idVendor != vid || desc.idProduct != pid) {
			continue;
		}

		printf("found, detach device:\n");
		ret = libusb_open(dev, &handler);
		if (ret < 0) {
			fprintf(stderr, "failed to open device: %s\n", libusb_strerror(ret));
			continue;
		}

		ret = libusb_detach_kernel_driver(handler, 0);
                if (ret < 0) {
			fprintf(stderr, "failed to detach device: %s\n", libusb_strerror(ret));
                }
                libusb_close(handler);
                handler = NULL;
	}

 out:
	libusb_free_device_list(devs, 1);
	return ;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <vendor id> <product id>", argv[0]);
		return -1;
	}

	int ret = libusb_init(&ctx);
	if (ret != LIBUSB_SUCCESS) {
		fprintf(stderr, "failed to init: %s\n", libusb_strerror(ret));
		return -1;
	}

        libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_DEBUG);
        //detach_device(argv[1], argv[2]);
        detach_device2(argv[1], argv[2]);
        libusb_exit(ctx);

        return 0;
}
