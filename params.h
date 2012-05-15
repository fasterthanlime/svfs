#ifndef _PARAMS_H_
#define _PARAMS_H_

#define FUSE_USE_VERSION 26
#define _XOPEN_SOURCE 700

#include <limits.h>
#include <stdio.h>
#include <time.h>

struct svfs_state {
    char *rootdir;
};

#define SVFS_DATA      ((struct svfs_state *) fuse_get_context()->private_data)

#endif
