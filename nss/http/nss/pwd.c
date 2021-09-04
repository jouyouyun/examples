#include "http.h"

static gint pw_idx = 0;

nss_setent(pw, pw_idx);
nss_endent(pw);
