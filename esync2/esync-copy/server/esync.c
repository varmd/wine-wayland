/*
 * eventfd-based synchronization objects
 *
 * Copyright (C) 2018 Zebediah Figura
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"


#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef HAVE_SYS_EVENTFD_H
# include <sys/eventfd.h>
#endif
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
 
#include "ntstatus.h"
#define WIN32_NO_STATUS
#include "windef.h"
#include "winternl.h"

#include "handle.h"
#include "request.h"
#include "file.h"
#include "esync.h"

int do_esync(void)
{

    return 0;

}



void esync_init(void)
{
    
}

struct esync
{
    struct object   obj;    /* object header */
    int             fd;     /* eventfd file descriptor */
    enum esync_type type;
    unsigned int    shm_idx;    /* index into the shared memory section */
};

static void esync_dump( struct object *obj, int verbose );
static int esync_get_esync_fd( struct object *obj, enum esync_type *type );
static unsigned int esync_map_access( struct object *obj, unsigned int access );
static void esync_destroy( struct object *obj );

const struct object_ops esync_ops =
{
    sizeof(struct esync),      /* size */
    &no_type,                  /* type */
    esync_dump,                /* dump */
    no_add_queue,              /* add_queue */
    NULL,                      /* remove_queue */
    NULL,                      /* signaled */
    esync_get_esync_fd,        /* get_esync_fd */
    NULL,                      /* fsync */
    NULL,                      /* satisfied */
    no_signal,                 /* signal */
    no_get_fd,                 /* get_fd */
    esync_map_access,          /* map_access */
    default_get_sd,            /* get_sd */
    default_set_sd,            /* set_sd */
    no_get_full_name,          /* get_full_name */
    no_lookup_name,            /* lookup_name */
    directory_link_name,       /* link_name */
    default_unlink_name,       /* unlink_name */
    no_open_file,              /* open_file */
    no_kernel_obj_list,        /* get_kernel_obj_list */
    no_close_handle,           /* close_handle */
    esync_destroy              /* destroy */
};


static void esync_dump( struct object *obj, int verbose )
{
    
}

static int esync_get_esync_fd( struct object *obj, enum esync_type *type )
{
    struct esync *esync = (struct esync *)obj;
    *type = esync->type;
    return esync->fd;
}

static unsigned int esync_map_access( struct object *obj, unsigned int access )
{
    return 0;
}

static void esync_destroy( struct object *obj )
{
    
}





struct semaphore
{
    int max;
    int count;
};
C_ASSERT(sizeof(struct semaphore) == 8);

struct mutex
{
    DWORD tid;
    int count;    /* recursion count */
};
C_ASSERT(sizeof(struct mutex) == 8);

struct event
{
    int signaled;
    int locked;
};
C_ASSERT(sizeof(struct event) == 8);



/* Create a file descriptor for an existing handle.
 * Caller must close the handle when it's done; it's not linked to an esync
 * server object in any way. */
int esync_create_fd( int initval, int flags )
{

}

/* Wake up a specific fd. */
void esync_wake_fd( int fd )
{

}

/* Wake up a server-side esync object. */
void esync_wake_up( struct object *obj )
{

}

void esync_clear( int fd )
{
    
}

static inline void small_pause(void)
{
#ifdef __i386__
    __asm__ __volatile__( "rep;nop" : : : "memory" );
#else
    __asm__ __volatile__( "" : : : "memory" );
#endif
}

/* Server-side event support. */
void esync_set_event( struct esync *esync )
{
    
}

void esync_reset_event( struct esync *esync )
{
    
}

DECL_HANDLER(create_esync)
{
    
}

DECL_HANDLER(open_esync)
{
    
}

/* Retrieve a file descriptor for an esync object which will be signaled by the
 * server. The client should only read from (i.e. wait on) this object. */
DECL_HANDLER(get_esync_fd)
{
    
}

/* Return the fd used for waiting on user APCs. */
DECL_HANDLER(get_esync_apc_fd)
{
    
}
