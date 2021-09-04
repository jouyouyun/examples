#include "http.h"

static gint sp_idx = 0;

nss_setent(sp, sp_idx);
nss_endent(sp);
