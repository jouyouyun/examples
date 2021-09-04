#include "http.h"

static gint gr_idx = 0;

nss_setent(gr, gr_idx);
nss_endent(gr);
