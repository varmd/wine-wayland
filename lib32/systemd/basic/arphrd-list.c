/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <errno.h>
#include <linux/if_arp.h>
#include <string.h>

#include "arphrd-list.h"
#include "macro.h"

static const struct arphrd_name* lookup_arphrd(register const char *str, register GPERF_LEN_TYPE len);

int arphrd_from_name(const char *name) {
  return 0;
}
