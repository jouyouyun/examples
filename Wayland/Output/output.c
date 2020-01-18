/**
 * Test wayland output
 *
 * Compile: gcc -Wall -g output.c `pkg-config --libs --cflags wayland-client`
 *
 **/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <wayland-client.h>

#define MIN(x,y) (((x)<(y))?(x):(y))

struct output_mode {
	struct wl_list link;

	uint32_t flags;
	int32_t width, height;
	int32_t refresh;
};

struct output_info {
	struct wl_output *core;
	struct wl_list link;

	int32_t version;

	struct {
		int32_t x, y;
		int32_t scale;
		int32_t phy_width, phy_height;
		enum wl_output_subpixel subpixel;
		enum wl_output_transform transform;
		char *make;
		char *model;
	} geometry;

	struct wl_list modes;
};

enum {
	ErrDisplayConnect = 1,
	ErrRegistryGet,
};

static void dump_output_infos(struct wl_list *list);
static void dump_output(void *data);

static void global_handler(void *data, struct wl_registry *reg, uint32_t id,
						   const char *interface, uint32_t version);
static void global_remove_handler(void *data, struct wl_registry *reg, uint32_t name);

static void add_output_info(struct wl_list *list, uint32_t id, uint32_t version);
static void destroy_output_infos(struct wl_list *list);
static void destroy_output_info(void *data);

static void output_handle_geometry(void *data, struct wl_output *output, int32_t x, int32_t y,
								   int32_t phy_width, int32_t phy_height, int32_t subpixel,
								   const char *make, const char *model, int32_t transform);
static void output_handle_mode(void *data, struct wl_output *output, uint32_t flags,
							   int32_t width, int32_t height, int32_t refresh);
static void output_handle_scale(void *data, struct wl_output *output, int32_t scale);
static void output_handle_done(void *data, struct wl_output *output);

static struct wl_display *dpy = NULL;
struct wl_registry *reg = NULL;
static const struct wl_registry_listener reg_listener = {
	global_handler,
	global_remove_handler,
};
static const struct wl_output_listener output_listener = {
	output_handle_geometry,
	output_handle_mode,
	output_handle_done,
	output_handle_scale,
};
static int output_done = 0;

int
main(int argc, char *argv[])
{
	struct wl_list output_infos;

	dpy = wl_display_connect(NULL);
	if (!dpy) {
		fprintf(stderr, "Failed to connect wayland display: %s\n", strerror(errno));
		return -ErrDisplayConnect;
	}

	wl_list_init(&output_infos);
	reg = wl_display_get_registry(dpy);
	if (!reg) {
		fprintf(stderr, "Failed to get registry: %s\n", strerror(errno));
		return -ErrRegistryGet;
	}

	wl_registry_add_listener(reg, &reg_listener, &output_infos);

	do {
		/* sleep(1); */
		output_done = 0;
		wl_display_roundtrip(dpy);
	} while(output_done);

	dump_output_infos(&output_infos);

	destroy_output_infos(&output_infos);

	return 0;
}

static void
dump_output_infos(struct wl_list *list)
{
	struct output_info *info;

	wl_list_for_each(info, list, link) {
		dump_output(info);
	}
}

static void
dump_output(void *data)
{
	struct output_info *info = data;
	struct output_mode *mode;
	const char *subpixel;
	const char *transform;

	printf("Output: '%s',  '%s'\n", info->geometry.make, info->geometry.model);
	printf("\tx: %d, y: %d, width: %d, height: %d\n", info->geometry.x, info->geometry.y,
		   info->geometry.phy_width, info->geometry.phy_height);

	switch (info->geometry.subpixel) {
	case WL_OUTPUT_SUBPIXEL_UNKNOWN:
		subpixel = "unknown";
		break;
	case WL_OUTPUT_SUBPIXEL_NONE:
		subpixel = "none";
		break;
	case WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB:
		subpixel = "horizontal rgb";
		break;
	case WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR:
		subpixel = "horizontal bgr";
		break;
	case WL_OUTPUT_SUBPIXEL_VERTICAL_RGB:
		subpixel = "vertical rgb";
		break;
	case WL_OUTPUT_SUBPIXEL_VERTICAL_BGR:
		subpixel = "vertical bgr";
		break;
	default:
		subpixel = "unexpected value";
		break;
	}

	switch (info->geometry.transform) {
	case WL_OUTPUT_TRANSFORM_NORMAL:
		transform = "normal";
		break;
	case WL_OUTPUT_TRANSFORM_90:
		transform = "90°";
		break;
	case WL_OUTPUT_TRANSFORM_180:
		transform = "180°";
		break;
	case WL_OUTPUT_TRANSFORM_270:
		transform = "270°";
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED:
		transform = "flipped";
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_90:
		transform = "flipped 90°";
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_180:
		transform = "flipped 180°";
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_270:
		transform = "flipped 270°";
		break;
	default:
		transform = "unexpected value";
		break;
	}

	printf("\tscale: %d, orientation: %s. transform: %s\n", info->geometry.scale, subpixel, transform);

	wl_list_for_each(mode, &info->modes, link) {
		printf("\tmode:\n");
		printf("\t\twidth: %d, height: %d, refresh: %.3f", mode->width, mode->height, mode->refresh/1000.0);
		printf("\t\tflags:");
		if (mode->flags & WL_OUTPUT_MODE_CURRENT) {
			printf(" [current]");
		}
		if (mode->flags & WL_OUTPUT_MODE_PREFERRED) {
			printf(" [preferred]");
		}
		printf("\n");
	}
}

static void
global_handler(void *data, struct wl_registry *reg, uint32_t id,
						   const char *interface, uint32_t version)
{
	// only handle output
	if (!strcmp(interface, "wl_output")) {
		struct wl_list *list = data;

		printf("[Event] output\n");
		add_output_info(list, id, version);
	}
}

static void
global_remove_handler(void *data, struct wl_registry *reg, uint32_t name)
{}

static void
add_output_info(struct wl_list *list, uint32_t id, uint32_t version)
{
	struct output_info *info = calloc(1, sizeof *info);

	info->version = MIN(version, 2);
	info->geometry.scale = 1;
	wl_list_init(&info->modes);

	info->core = wl_registry_bind(reg, id, &wl_output_interface, info->version);
	wl_output_add_listener(info->core, &output_listener, info);

	output_done = 1;
	wl_list_insert(list, &info->link);
}

static void
destroy_output_infos(struct wl_list *list)
{
	struct output_info *info;

	wl_list_for_each(info, list, link) {
		destroy_output_info(info);
	}
}

static void
destroy_output_info(void *data)
{
	struct output_info *info = data;
	struct output_mode *mode, *tmp;

	wl_output_destroy(info->core);

	if (info->geometry.make) {
		free(info->geometry.make);
	}
	if (info->geometry.model) {
		free(info->geometry.model);
	}

	wl_list_for_each_safe(mode, tmp, &info->modes, link) {
		wl_list_remove(&mode->link);
		free(mode);
	}
}

static void
output_handle_geometry(void *data, struct wl_output *output, int32_t x, int32_t y,
								   int32_t phy_width, int32_t phy_height, int32_t subpixel,
								   const char *make, const char *model, int32_t transform)
{
	struct output_info *info = data;

	printf("Make output geometry\n");
	info->geometry.x = x;
	info->geometry.y = y;
	info->geometry.phy_width = phy_width;
	info->geometry.phy_height = phy_height;
	info->geometry.subpixel = subpixel;
	info->geometry.make = strdup(make);
	info->geometry.model = strdup(model);
	info->geometry.transform = transform;
}

static void
output_handle_mode(void *data, struct wl_output *output, uint32_t flags,
							   int32_t width, int32_t height, int32_t refresh)
{
	struct output_info *info = data;
	struct output_mode *mode = calloc(1, sizeof *mode);

	printf("Make output mode\n");
	mode->flags = flags;
	mode->width = width;
	mode->height = height;
	mode->refresh = refresh;

	wl_list_insert(info->modes.prev, &mode->link);
}

static void
output_handle_scale(void *data, struct wl_output *output, int32_t scale)
{
	struct output_info *info = data;

	printf("Make output scale\n");
	info->geometry.scale = scale;
}

static void
output_handle_done(void *data, struct wl_output *output)
{
	printf("Make output done\n");
	/* output_done = 1; */
}
