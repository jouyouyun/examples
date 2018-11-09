#include <glib.h>
#include <signal.h>

struct raw_coord {
	double x, y;
};

static guint timeid = 0;

static gboolean
timeout_handler(gpointer data)
{
	struct raw_coord *coord = (struct raw_coord*)data;
	g_print("Timeout recieved: %u, data: (%f, %f)\n", timeid, coord->x, coord->y);
	timeid = 0;
	return FALSE;
}

static void
destroy_handler(gpointer data)
{
	struct raw_coord *coord = (struct raw_coord*)data;
	g_print("Timeout destroy: %u, data: (%f, %f)\n", timeid, coord->x, coord->y);
	timeid = 0;
	return ;
}

static void
sig_handler(int sig)
{
	g_print("Recieved signal: %d, cancel timeout: %u\n", sig, timeid);
	if (timeid == 0) {
		return;
	}
	g_source_remove(timeid);
}

int
main(int argc, char *argv[])
{
	signal(SIGALRM, sig_handler);
	g_print("Send SIGALRM to destroy timeout\n");

	struct raw_coord coord = {
		.x = 5,
		.y = 10,
	};
	timeid = g_timeout_add_full(G_PRIORITY_DEFAULT, 10 * 1000,
				    timeout_handler, &coord, destroy_handler);
	g_print("Timeout id: %u\n", timeid);

	g_main_loop_run(g_main_loop_new(NULL, TRUE));
}
