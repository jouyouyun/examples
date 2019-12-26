#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <glib.h>
#include <glib/gprintf.h>

#define DEFAULT_CPU_CORE 4
#define MAX_BUF_SIZE 1024

static int create_thread_pool();
static void handle_data(gpointer data, gpointer user_data);
static int get_cpu_cores();

static GThreadPool *_pool = NULL;

int
main()
{
	int i = 0, j = 0;
	int ret = 0;
	gchar buf[MAX_BUF_SIZE] = {0};
	GError *error = NULL;
	GMainLoop *loop = NULL;

	ret = create_thread_pool();
	if (ret)
		return ret;

	for (; i < 5; i++) {
		j = 0;
		for (; j < 10; j++) {
			memset(buf, 0, MAX_BUF_SIZE);
			g_sprintf(buf, "data: %d - %d", i, j);
			g_thread_pool_push(_pool, (gpointer)g_strdup(buf), &error);
			if (error) {
				g_warning("failed to push data: %s, error: %s\n", buf, error->message);
				g_error_free(error);
				error = NULL;
			}
		}
		sleep(3);
	}

	loop = g_main_loop_new(NULL, FALSE);
	if (!loop) {
		g_error("failed to new loop\n");
		return -1;
	}

	g_main_loop_run(loop);
	g_thread_pool_free(_pool, TRUE, TRUE);
	return 0;
}

static int
create_thread_pool()
{
	GError *error = NULL;

	_pool = g_thread_pool_new(handle_data, NULL,
							 get_cpu_cores(), TRUE, &error);
	if (error) {
		g_error("faield to create thread pool: %s\n", error->message);
		g_error_free(error);
		error = NULL;
		return -1;
	}

	return 0;
}

static void
handle_data(gpointer data, gpointer user_data)
{
	if (data == NULL)
		return;

	char *str = (char*)data;
	g_print("Will handle: %s\n", str);
	g_free(str);
}

static int
get_cpu_cores()
{
	// TODO(jouyouyun): implement by parse /proc/cpuinfo
	return DEFAULT_CPU_CORE;
}
