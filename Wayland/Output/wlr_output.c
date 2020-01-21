/**
 *
 * compile: 'gcc -Wall -g wlr_output.c `pkg-config --libs --cflags wayland-client wayland-server` -lm'
 *
 **/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <wayland-client.h>
#include <wayland-util.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

struct output_manager {
	struct wl_display *display;
	struct wl_global *global;
	struct wl_list resources;

	char make[54];
	char model[16];
	int32_t x, y;
	int32_t phys_width, phys_height;
	int32_t width, height;
	int32_t refresh; // mHz, may be zero

	float scale;
	enum wl_output_subpixel subpixel;
	enum wl_output_transform transform;
};

#define OUTPUT_VERSION 3

static void output_handle_release(struct wl_client *client,
								  struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct wl_output_interface output_impl = {
	.release = output_handle_release,
};

struct output_manager *output_from_resource(struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &wl_output_interface,
								   &output_impl));
	return wl_resource_get_user_data(resource);
}

static void send_geometry(struct wl_resource *resource) {
	struct output_manager *manager = output_from_resource(resource);
	wl_output_send_geometry(resource, 0, 0,
							manager->phys_width, manager->phys_height, manager->subpixel,
							manager->make, manager->model, manager->transform);
}

static void send_current_mode(struct wl_resource *resource) {
	    struct output_manager *manager = output_from_resource(resource);
		wl_output_send_mode(resource, WL_OUTPUT_MODE_CURRENT, manager->width,
							manager->height, manager->refresh);
}

static void send_scale(struct wl_resource *resource) {
	struct output_manager *manager = output_from_resource(resource);
	uint32_t version = wl_resource_get_version(resource);
	if (version >= WL_OUTPUT_SCALE_SINCE_VERSION) {
		wl_output_send_scale(resource, (uint32_t)ceil(manager->scale));
	}
}

static void send_done(struct wl_resource *resource) {
	uint32_t version = wl_resource_get_version(resource);
	if (version >= WL_OUTPUT_DONE_SINCE_VERSION) {
		wl_output_send_done(resource);
	}
}

static void output_handle_resource_destroy(struct wl_resource *resource) {
	wl_list_remove(wl_resource_get_link(resource));
}

static void
output_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
	if (!client) {
		fprintf(stderr, "Invalid client\n");
		return;
	}

	printf("[%s] will bind output: %u, %u\n", __func__, version, id);
	struct output_manager *manager = data;
	struct wl_resource *resource = wl_resource_create(client, &wl_output_interface, version, id);
	if (resource == NULL) {
		wl_client_post_no_memory(client);
		fprintf(stderr, "failed to create resource: %s\n", strerror(errno));
		return;
	}

	wl_resource_set_implementation(resource, &output_impl, manager,
								   output_handle_resource_destroy);
	wl_list_insert(&manager->resources, wl_resource_get_link(resource));

	printf("[%s] will set resource\n", __func__);
	send_geometry(resource);
	send_current_mode(resource);
	send_scale(resource);
	send_done(resource);
}

void
output_create_global(struct output_manager *manager)
{
	if (manager->global) {
		return;
	}

	manager->global = wl_global_create(manager->display, &wl_output_interface, OUTPUT_VERSION,
									   manager, output_bind);
	if (manager->global == NULL) {
		fprintf(stderr, "failed to create global: %s\n", strerror(errno));
		return;
	}
}

void
output_destroy_global(struct output_manager *manager) {
	if (manager->global == NULL) {
		return;
	}
	// Make all output resources inert
	struct wl_resource *resource, *tmp;
	wl_resource_for_each_safe(resource, tmp, &manager->resources) {
		wl_resource_set_user_data(resource, NULL);
		wl_list_remove(wl_resource_get_link(resource));
		wl_list_init(wl_resource_get_link(resource));
	}
	wl_global_destroy(manager->global);
	manager->global = NULL;
}

int
main(int argc, char *argv[])
{
	if (argc!=2) {
		fprintf(stderr, "Usage: %s <wayland socket>\n", argv[0]);
		return 0;
	}

	struct wl_display *disp = wl_display_connect(argv[1]);
	if (!disp) {
		fprintf(stderr, "Failed to connect display: %s\n", strerror(errno));
		return 0;
	}

	struct output_manager manager = {
		.display = disp,
		.x = 0,
		.y = 0,
		.width = 1024,
		.height = 768,
		.refresh = 60 * 1000,
		.scale = 1.0,
		.subpixel = 0,
		.transform = 0,
	};
	wl_list_init(&manager.resources);

	printf("Start to create global\n");
	output_create_global(&manager);

	printf("Start to run\n");
	/* wl_display_dispatch(manager.display); */
	wl_display_run(manager.display);

	printf("Start to destroy\n");
	output_destroy_global(&manager);
	wl_display_disconnect(manager.display);

	return 0;
}
