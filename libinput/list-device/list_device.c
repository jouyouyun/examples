#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <poll.h>
#include <libinput.h>
#include <libudev.h>

static int open_restricted(const char *path, int flags, void *user_data);
static void close_restricted(int fd, void *user_data);
static struct libinput *open_from_udev(char *seat, void *data, int verbose);
static void list_device();
static void monitor_event();
static void handle_event();
static void dump_device(struct libinput_device *dev);

#define SCREEN_WIDTH 100
#define SCREEN_HEIGHT 100

static const struct libinput_interface li_ifc = {
	.open_restricted = open_restricted,
	.close_restricted = close_restricted,
};
static struct libinput *handler;

int
main(int argc, char *argv[])
{
	handler = open_from_udev("seat0", NULL, 1);
	if (!handler) {
		return -1;
	}

	list_device();
	printf("Start monitor\n");
	monitor_event();
	return 0;
}

static void
list_device()
{
	libinput_dispatch(handler);
	struct libinput_event *ev;
	while((ev = libinput_get_event(handler))) {
		enum libinput_event_type ty = libinput_event_get_type(ev);
		if (ty != LIBINPUT_EVENT_DEVICE_ADDED) {
			break;
		}
		struct libinput_device *dev = libinput_event_get_device(ev);
		if (!dev) {
			continue;
		}
		dump_device(dev);
		libinput_event_destroy(ev);
		libinput_dispatch(handler);
	}
}

static void
monitor_event()
{
	struct pollfd fds;
	fds.fd = libinput_get_fd(handler);
	fds.events = POLLIN;
	fds.revents = 0;

	while(poll(&fds, 1, -1) > -1) {
		handle_event(handler);
	}
}

static void
handle_event()
{
	libinput_dispatch(handler);
	struct libinput_event *ev;
	while((ev = libinput_get_event(handler))) {
		enum libinput_event_type ty = libinput_event_get_type(ev);
		if (ty == LIBINPUT_EVENT_TOUCH_DOWN ||
		    ty == LIBINPUT_EVENT_TOUCH_MOTION) {
			struct libinput_event_touch *touch = libinput_event_get_touch_event(ev);
			printf("[Touch] type(%d), Absolute(%f x %f)\n", ty,
			       libinput_event_touch_get_y(touch), libinput_event_touch_get_y(touch));
			printf("[Touch] type(%d), Transformed(%f x %f)\n", ty,
			       libinput_event_touch_get_x_transformed(touch, SCREEN_WIDTH),
			       libinput_event_touch_get_y_transformed(touch, SCREEN_HEIGHT));
		}
		libinput_event_destroy(ev);
		libinput_dispatch(handler);
	}
}

static void
dump_device(struct libinput_device *dev)
{
	printf("========  Dump  ========\n");
	printf("Name:%s\n", libinput_device_get_name(dev));
	printf("Output Name: %s\n", libinput_device_get_output_name(dev));
	printf("System Name: %s\n", libinput_device_get_sysname(dev));
	printf("ID product(%u), vendor(%u)\n",
	       libinput_device_get_id_product(dev),
	       libinput_device_get_id_vendor(dev));
	double width, height;
	libinput_device_get_size(dev, &width, &height);
	printf("Size: %f x %f\n", width, height);
	printf("\n");
}

static struct libinput*
open_from_udev(char *seat, void *data, int verbose)
{
	struct udev *dev = udev_new();
	if (!dev) {
		fprintf(stderr, "Failed to initialize udev: %s\n",
			strerror(errno));
		return NULL;
	}

	struct libinput *li = libinput_udev_create_context(&li_ifc, data, dev);
	if (!li) {
		fprintf(stderr, "Failed to initialize libinput context from udev: %s\n",
			strerror(errno));
		udev_unref(dev);
		return NULL;
	}

	if (verbose) {
		libinput_log_set_priority(li, LIBINPUT_LOG_PRIORITY_DEBUG);
	}

	if (!seat) {
		seat = "seat0";
	}

	if (libinput_udev_assign_seat(li, seat)){
		fprintf(stderr, "Failed to assign seat: %s\n", strerror(errno));
		libinput_unref(li);
		udev_unref(dev);
		return NULL;
	}
	return li;
}

static int
open_restricted(const char *path, int flags, void *user_data)
{
	int fd = open(path, flags);
	if (fd < 0) {
		fprintf(stderr, "Failed to open '%s': %s\n",
			path, strerror(errno));
		return -errno;
	}
	return fd;
}

static void
close_restricted(int fd, void *user_data)
{
	close(fd);
}
