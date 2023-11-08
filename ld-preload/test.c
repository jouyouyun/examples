#include <glib.h>

int main()
{
		gpointer data = NULL;

		data = g_malloc(10);
		if (!data) {
				g_print("malloc failed\n");
				return 0;
		}

		g_print("malloc success\n");
		g_free(data);
		return 0;
}
