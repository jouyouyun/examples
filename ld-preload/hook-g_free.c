#define _GNU_SOURCE
#include <dlfcn.h>
#include <glib.h>

void (*orig_g_free)(gpointer mem);

void g_free(gpointer mem)
{
  if (!orig_g_free)
    orig_g_free = dlsym(RTLD_NEXT, "g_free");

  g_print("Hook!!!\n");
  orig_g_free(mem);
}
