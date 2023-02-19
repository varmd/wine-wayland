/*
 * futex-based synchronization objects
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

#if 0
#pragma makedep unix
#endif


#include "config.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#endif
#include <unistd.h>
#include <stdint.h>

#include "ntstatus.h"
#define WIN32_NO_STATUS
#define NONAMELESSUNION
#include "windef.h"
#include "winternl.h"
#include "wine/debug.h"
#include "wine/server.h"

#include "unix_private.h"
#include "fsync.h"




WINE_DEFAULT_DEBUG_CHANNEL(fsync);




#include "pshpack4.h"
#include "poppack.h"

/* futex_waitv interface */

#ifndef __NR_futex_waitv
# define __NR_futex_waitv 449
#endif

#ifndef FUTEX_32
# define FUTEX_32 2
struct futex_waitv {
    uint64_t   val;
    uint64_t   uaddr;
    uint32_t   flags;
    uint32_t __reserved;
};
#endif

#define u64_to_ptr(x) (void *)(uintptr_t)(x)

struct timespec64
{
    long long tv_sec;
    long long tv_nsec;
};

static LONGLONG update_timeout( ULONGLONG end )
{
    LARGE_INTEGER now;
    LONGLONG timeleft;

    NtQuerySystemTime( &now );
    timeleft = end - now.QuadPart;
    if (timeleft < 0) timeleft = 0;
    return timeleft;
}

static inline void futex_vector_set( struct futex_waitv *waitv, int *addr, int val )
{
    waitv->uaddr = (uintptr_t) addr;
    waitv->val = val;
    waitv->flags = FUTEX_32;
    waitv->__reserved = 0;
}

static inline int futex_wait_multiple( const struct futex_waitv *futexes,
        int count, const ULONGLONG *end )
{
   if (end)
   {
        struct timespec64 timeout;
        ULONGLONG tmp = *end - SECS_1601_TO_1970 * TICKSPERSEC;
        timeout.tv_sec = tmp / (ULONGLONG)TICKSPERSEC;
        timeout.tv_nsec = (tmp % TICKSPERSEC) * 100;

        return syscall( __NR_futex_waitv, futexes, count, 0, &timeout, CLOCK_REALTIME );
   }
   else
   {
        return syscall( __NR_futex_waitv, futexes, count, 0, NULL, 0 );
   }
}

static inline int futex_wake( int *addr, int val )
{
    return syscall( __NR_futex, addr, 1, val, NULL, 0, 0 );
}

static inline int futex_wait( int *addr, int val, const ULONGLONG *end )
{
    if (end)
    {
        LONGLONG timeleft = update_timeout( *end );
        struct timespec timeout;
        timeout.tv_sec = timeleft / (ULONGLONG)TICKSPERSEC;
        timeout.tv_nsec = (timeleft % TICKSPERSEC) * 100;
        return syscall( __NR_futex, addr, 0, val, &timeout, 0, 0 );
    }
    else
    {
        return syscall( __NR_futex, addr, 0, val, NULL, 0, 0 );
    }
}

static inline int get_fdzero(void)
{
    static int fd = -1;

    if (MAP_ANON == 0 && fd == -1)
    {
        if ((fd = open( "/dev/zero", O_RDONLY )) == -1)
        {
            perror( "/dev/zero: open" );
            exit(1);
        }
    }
    return fd;
}
/***********************************************************************
 *		wine_anon_mmap
 *
 * Portable wrapper for anonymous mmaps
 */
static void *wine_anon_mmap( void *start, size_t size, int prot, int flags )
{
#ifdef MAP_SHARED
    flags &= ~MAP_SHARED;
#endif

    /* Linux EINVAL's on us if we don't pass MAP_PRIVATE to an anon mmap */
    flags |= MAP_PRIVATE | MAP_ANON;

    if (!(flags & MAP_FIXED))
    {
#ifdef MAP_TRYFIXED
        /* If available, this will attempt a fixed mapping in-kernel */
        flags |= MAP_TRYFIXED;
#endif

    }
    return mmap( start, size, prot, flags, get_fdzero(), 0 );
}

static int global_fsync_active = 0;

void activate_fsync(void) {
  global_fsync_active = 1;
}




int do_fsync(void)
{

  if(!global_fsync_active)
    return 0;

#ifdef __linux__
    static int do_fsync_cached = -1;

    if (do_fsync_cached == -1)
    {
        syscall( __NR_futex_waitv, NULL, 0, 0, NULL, 0 );
        do_fsync_cached = getenv("WINEFSYNC") && atoi(getenv("WINEFSYNC")) && errno != ENOSYS;
    }

    return do_fsync_cached;
#else
    static int once;
    if (!once++)
        FIXME("futexes not supported on this platform.\n");
    return 0;
#endif
}

struct fsync
{
    enum fsync_type type;
    void *shm;              /* pointer to shm section */
};

struct semaphore
{
    int count;
    int max;
};
C_ASSERT(sizeof(struct semaphore) == 8);

struct event
{
    int signaled;
    int unused;
};
C_ASSERT(sizeof(struct event) == 8);

struct mutex
{
    int tid;
    int count;  /* recursion count */
};
C_ASSERT(sizeof(struct mutex) == 8);

static char shm_name[29];
static int shm_fd;
static void **shm_addrs;
static int shm_addrs_size;  /* length of the allocated shm_addrs array */
static long pagesize;

static pthread_mutex_t shm_addrs_section = PTHREAD_MUTEX_INITIALIZER;;

#if 0
static RTL_CRITICAL_SECTION_DEBUG shm_addrs_debug =
{
    0, 0, &shm_addrs_section,
    { &shm_addrs_debug.ProcessLocksList, &shm_addrs_debug.ProcessLocksList },
      0, 0, { (DWORD_PTR)(__FILE__ ": shm_addrs_section") }
};
static RTL_CRITICAL_SECTION shm_addrs_section = { &shm_addrs_debug, -1, 0, 0, 0, 0 };
#endif

static void *get_shm( unsigned int idx )
{
    int entry  = (idx * 8) / pagesize;
    int offset = (idx * 8) % pagesize;
    void *ret;

    pthread_mutex_lock(&shm_addrs_section);


    //printf("Entry is %d \n", entry);

    if (entry >= shm_addrs_size)
    {
        shm_addrs_size = entry + 1;

        //Realloc seems to cause crashes
        //for unknown reasons realloc causes crashes
        //use large amount of initial array to avoid crashes
        //TODO
        //probably new addresses needs to be set to zero like in server/fsync.c
//        printf("Realloc on %d \n", shm_addrs_size);
        shm_addrs = realloc( shm_addrs, (entry + 1) * sizeof(shm_addrs[0]) );
        if (!shm_addrs) {
          ERR("Failed to grow shm_addrs array to size %d.\n", shm_addrs_size);
//          printf("Failed to grow shm_addrs array to size %d.\n", shm_addrs_size);
          exit(1);
        }
        //if (!(shm_addrs = RtlReAllocateHeap( GetProcessHeap(), HEAP_ZERO_MEMORY,
        //        shm_addrs, shm_addrs_size * sizeof(shm_addrs[0]) )))
        //        shm_addrs, shm_addrs_size * sizeof(shm_addrs[0]) )))

    }

    if (!shm_addrs[entry])
    {
        void *addr = mmap( NULL, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, entry * pagesize );
        if (addr == (void *)-1)
            ERR("Failed to map page %d (offset %#lx).\n", entry, entry * pagesize);

//        TRACE("Mapping page %d at %p.\n", entry, addr);

        if (__sync_val_compare_and_swap( &shm_addrs[entry], 0, addr ))
            munmap( addr, pagesize ); /* someone beat us to it */
    }

    ret = (void *)((unsigned long)shm_addrs[entry] + offset);

    pthread_mutex_unlock(&shm_addrs_section);

    return ret;
}

/* We'd like lookup to be fast. To that end, we use a static list indexed by handle.
 * This is copied and adapted from the fd cache code. */

#define FSYNC_LIST_BLOCK_SIZE  (65536 / sizeof(struct fsync))
#define FSYNC_LIST_ENTRIES     256

static struct fsync *fsync_list[FSYNC_LIST_ENTRIES];
static struct fsync fsync_list_initial_block[FSYNC_LIST_BLOCK_SIZE];

static inline UINT_PTR handle_to_index( HANDLE handle, UINT_PTR *entry )
{
    UINT_PTR idx = (((UINT_PTR)handle) >> 2) - 1;
    *entry = idx / FSYNC_LIST_BLOCK_SIZE;
    return idx % FSYNC_LIST_BLOCK_SIZE;
}

static struct fsync *add_to_list( HANDLE handle, enum fsync_type type, void *shm )
{
    UINT_PTR entry, idx = handle_to_index( handle, &entry );

    if (entry >= FSYNC_LIST_ENTRIES)
    {
        FIXME( "too many allocated handles, not caching %p\n", handle );
        return FALSE;
    }

    if (!fsync_list[entry])  /* do we need to allocate a new block of entries? */
    {
        if (!entry) fsync_list[0] = fsync_list_initial_block;
        else
        {
            void *ptr = wine_anon_mmap( NULL, FSYNC_LIST_BLOCK_SIZE * sizeof(struct fsync),
                                        PROT_READ | PROT_WRITE, 0 );
            if (ptr == MAP_FAILED) return FALSE;
            fsync_list[entry] = ptr;
        }
    }

    if (!__sync_val_compare_and_swap((int *)&fsync_list[entry][idx].type, 0, type ))
        fsync_list[entry][idx].shm = shm;

    return &fsync_list[entry][idx];
}

static struct fsync *get_cached_object( HANDLE handle )
{
    UINT_PTR entry, idx = handle_to_index( handle, &entry );

    if (entry >= FSYNC_LIST_ENTRIES || !fsync_list[entry]) return NULL;
    if (!fsync_list[entry][idx].type) return NULL;

    return &fsync_list[entry][idx];
}

/* Gets an object. This is either a proper fsync object (i.e. an event,
 * semaphore, etc. created using create_fsync) or a generic synchronizable
 * server-side object which the server will signal (e.g. a process, thread,
 * message queue, etc.) */
static NTSTATUS get_object( HANDLE handle, struct fsync **obj )
{
    NTSTATUS ret = STATUS_SUCCESS;
    unsigned int shm_idx = 0;
    enum fsync_type type;

    if ((*obj = get_cached_object( handle ))) return STATUS_SUCCESS;

    if ((INT_PTR)handle < 0)
    {
        /* We can deal with pseudo-handles, but it's just easier this way */
        return STATUS_NOT_IMPLEMENTED;
    }

    /* We need to try grabbing it from the server. */
    SERVER_START_REQ( get_fsync_idx )
    {
        req->handle = wine_server_obj_handle( handle );
        if (!(ret = wine_server_call( req )))
        {
            shm_idx = reply->shm_idx;
            type    = reply->type;
        }
    }
    SERVER_END_REQ;

    if (ret)
    {
        WARN("Failed to retrieve shm index %d for handle %p, status %#x.\n", shm_idx, handle, ret);
        *obj = NULL;
        return ret;
    }

//    TRACE("Got shm index %d for handle %p.\n", shm_idx, handle);

    *obj = add_to_list( handle, type, get_shm( shm_idx ) );
    return ret;
}

NTSTATUS fsync_close( HANDLE handle )
{
    UINT_PTR entry, idx = handle_to_index( handle, &entry );

//    TRACE("%p.\n", handle);

    if (entry < FSYNC_LIST_ENTRIES && fsync_list[entry])
    {
        if (__atomic_exchange_n( &fsync_list[entry][idx].type, 0, __ATOMIC_SEQ_CST ))
            return STATUS_SUCCESS;
    }

    return STATUS_INVALID_HANDLE;
}

static NTSTATUS create_fsync( enum fsync_type type, HANDLE *handle,
    ACCESS_MASK access, const OBJECT_ATTRIBUTES *attr, int low, int high )
{
    NTSTATUS ret;
    data_size_t len;
    struct object_attributes *objattr;
    unsigned int shm_idx;

    if ((ret = alloc_object_attributes( attr, &objattr, &len ))) return ret;

    SERVER_START_REQ( create_fsync )
    {
        req->access = access;
        req->low    = low;
        req->high   = high;
        req->type   = type;
        wine_server_add_data( req, objattr, len );
        ret = wine_server_call( req );
        if (!ret || ret == STATUS_OBJECT_NAME_EXISTS)
        {
            *handle = wine_server_ptr_handle( reply->handle );
            shm_idx = reply->shm_idx;
            type    = reply->type;
        }
    }
    SERVER_END_REQ;

    if (!ret || ret == STATUS_OBJECT_NAME_EXISTS)
    {
        add_to_list( *handle, type, get_shm( shm_idx ));
//        TRACE("-> handle %p, shm index %d.\n", *handle, shm_idx);
    }

    free( objattr );
    return ret;
}

static NTSTATUS open_fsync( enum fsync_type type, HANDLE *handle,
    ACCESS_MASK access, const OBJECT_ATTRIBUTES *attr )
{
    NTSTATUS ret;
    unsigned int shm_idx;

    SERVER_START_REQ( open_fsync )
    {
        req->access     = access;
        req->attributes = attr->Attributes;
        req->rootdir    = wine_server_obj_handle( attr->RootDirectory );
        req->type       = type;
        if (attr->ObjectName)
            wine_server_add_data( req, attr->ObjectName->Buffer, attr->ObjectName->Length );
        if (!(ret = wine_server_call( req )))
        {
            *handle = wine_server_ptr_handle( reply->handle );
            type = reply->type;
            shm_idx = reply->shm_idx;
        }
    }
    SERVER_END_REQ;

    if (!ret)
    {
        add_to_list( *handle, type, get_shm( shm_idx ) );

//        TRACE("-> handle %p, shm index %u.\n", *handle, shm_idx);
    }
    return ret;
}

void fsync_init(void)
{
    printf("fsync starting \n");

    struct stat st;

    if (!do_fsync())
    {
        /* make sure the server isn't running with WINEFSYNC */
        HANDLE handle;
        NTSTATUS ret;

        ret = create_fsync( 0, &handle, 0, NULL, 0, 0 );
        if (ret != STATUS_NOT_IMPLEMENTED)
        {
            ERR("Server is running with WINEFSYNC but this process is not, please enable WINEFSYNC or restart wineserver.\n");
            exit(1);
        }

        return;
    }

    if (stat( config_dir, &st ) == -1)
        ERR("Cannot stat %s\n", config_dir);

    if (st.st_ino != (unsigned long)st.st_ino)
        sprintf( shm_name, "/wine-%lx%08lx-fsync", (unsigned long)((unsigned long long)st.st_ino >> 32), (unsigned long)st.st_ino );
    else
        sprintf( shm_name, "/wine-%lx-fsync", (unsigned long)st.st_ino );

    if ((shm_fd = shm_open( shm_name, O_RDWR, 0644 )) == -1)
    {
        /* probably the server isn't running with WINEFSYNC, tell the user and bail */
        if (errno == ENOENT)
            ERR("Failed to open fsync shared memory file; make sure no stale wineserver instances are running without WINEFSYNC.\n");
        else
            ERR("Failed to initialize shared memory: %s\n", strerror( errno ));
        exit(1);
    }

    pagesize = sysconf( _SC_PAGESIZE );

    //shm_addrs = RtlAllocateHeap( GetProcessHeap(), HEAP_ZERO_MEMORY, 128 * sizeof(shm_addrs[0]) );
    //shm_addrs = calloc( 128, 128 * sizeof(shm_addrs[0]) );
    shm_addrs = calloc( 1280000, sizeof(shm_addrs[0]) );
    if(!shm_addrs) {
      exit(1);
    }
    shm_addrs_size = 1280000;
//    printf("Alloc on %p %d %ld \n", shm_addrs, shm_addrs_size, sizeof(shm_addrs[0]));
}

NTSTATUS fsync_create_semaphore( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr, LONG initial, LONG max )
{
//    TRACE("name %s, initial %d, max %d.\n",
//        attr ? debugstr_us(attr->ObjectName) : "<no name>", initial, max);

    return create_fsync( FSYNC_SEMAPHORE, handle, access, attr, initial, max );
}

NTSTATUS fsync_open_semaphore( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr )
{
//    TRACE("name %s.\n", debugstr_us(attr->ObjectName));

    return open_fsync( FSYNC_SEMAPHORE, handle, access, attr );
}

NTSTATUS fsync_release_semaphore( HANDLE handle, ULONG count, ULONG *prev )
{
    struct fsync *obj;
    struct semaphore *semaphore;
    ULONG current;
    NTSTATUS ret;

//    TRACE("%p, %d, %p.\n", handle, count, prev);

    if ((ret = get_object( handle, &obj ))) return ret;
    semaphore = obj->shm;

    do
    {
        current = semaphore->count;
        if (count + current > semaphore->max)
            return STATUS_SEMAPHORE_LIMIT_EXCEEDED;
    } while (__sync_val_compare_and_swap( &semaphore->count, current, count + current ) != current);

    if (prev) *prev = current;

    futex_wake( &semaphore->count, INT_MAX );

    return STATUS_SUCCESS;
}

NTSTATUS fsync_query_semaphore( HANDLE handle, SEMAPHORE_INFORMATION_CLASS class,
    void *info, ULONG len, ULONG *ret_len )
{
    struct fsync *obj;
    struct semaphore *semaphore;
    SEMAPHORE_BASIC_INFORMATION *out = info;
    NTSTATUS ret;

//    TRACE("%p, %u, %p, %u, %p.\n", handle, class, info, len, ret_len);

    if (class != SemaphoreBasicInformation)
    {
        FIXME("(%p,%d,%u) Unknown class\n", handle, class, len);
        return STATUS_INVALID_INFO_CLASS;
    }

    if ((ret = get_object( handle, &obj ))) return ret;
    semaphore = obj->shm;

    out->CurrentCount = semaphore->count;
    out->MaximumCount = semaphore->max;
    if (ret_len) *ret_len = sizeof(*out);

    return STATUS_SUCCESS;
}

NTSTATUS fsync_create_event( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr, EVENT_TYPE event_type, BOOLEAN initial )
{
    enum fsync_type type = (event_type == SynchronizationEvent ? FSYNC_AUTO_EVENT : FSYNC_MANUAL_EVENT);

//    TRACE("name %s, %s-reset, initial %d.\n",
//        attr ? debugstr_us(attr->ObjectName) : "<no name>",
//        event_type == NotificationEvent ? "manual" : "auto", initial);

    return create_fsync( type, handle, access, attr, initial, 0xdeadbeef );
}

NTSTATUS fsync_open_event( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr )
{
//    TRACE("name %s.\n", debugstr_us(attr->ObjectName));

    return open_fsync( FSYNC_AUTO_EVENT, handle, access, attr );
}

NTSTATUS fsync_set_event( HANDLE handle, LONG *prev )
{
    struct event *event;
    struct fsync *obj;
    LONG current;
    NTSTATUS ret;

//    TRACE("%p.\n", handle);

    if ((ret = get_object( handle, &obj ))) return ret;
    event = obj->shm;

    if (!(current = __atomic_exchange_n( &event->signaled, 1, __ATOMIC_SEQ_CST )))
        futex_wake( &event->signaled, INT_MAX );

    if (prev) *prev = current;

    return STATUS_SUCCESS;
}

NTSTATUS fsync_reset_event( HANDLE handle, LONG *prev )
{
    struct event *event;
    struct fsync *obj;
    LONG current;
    NTSTATUS ret;

//    TRACE("%p.\n", handle);

    if ((ret = get_object( handle, &obj ))) return ret;
    event = obj->shm;

    current = __atomic_exchange_n( &event->signaled, 0, __ATOMIC_SEQ_CST );

    if (prev) *prev = current;

    return STATUS_SUCCESS;
}

NTSTATUS fsync_pulse_event( HANDLE handle, LONG *prev )
{
    struct event *event;
    struct fsync *obj;
    LONG current;
    NTSTATUS ret;

//    TRACE("%p.\n", handle);

    if ((ret = get_object( handle, &obj ))) return ret;
    event = obj->shm;

    /* This isn't really correct; an application could miss the write.
     * Unfortunately we can't really do much better. Fortunately this is rarely
     * used (and publicly deprecated). */
    if (!(current = __atomic_exchange_n( &event->signaled, 1, __ATOMIC_SEQ_CST )))
        futex_wake( &event->signaled, INT_MAX );

    /* Try to give other threads a chance to wake up. Hopefully erring on this
     * side is the better thing to do... */
    NtYieldExecution();

    __atomic_store_n( &event->signaled, 0, __ATOMIC_SEQ_CST );

    if (prev) *prev = current;

    return STATUS_SUCCESS;
}

NTSTATUS fsync_query_event( HANDLE handle, EVENT_INFORMATION_CLASS class,
    void *info, ULONG len, ULONG *ret_len )
{
    struct event *event;
    struct fsync *obj;
    EVENT_BASIC_INFORMATION *out = info;
    NTSTATUS ret;

//    TRACE("%p, %u, %p, %u, %p.\n", handle, class, info, len, ret_len);

    if (class != EventBasicInformation)
    {
        FIXME("(%p,%d,%u) Unknown class\n", handle, class, len);
        return STATUS_INVALID_INFO_CLASS;
    }

    if ((ret = get_object( handle, &obj ))) return ret;
    event = obj->shm;

    out->EventState = event->signaled;
    out->EventType = (obj->type == FSYNC_AUTO_EVENT ? SynchronizationEvent : NotificationEvent);
    if (ret_len) *ret_len = sizeof(*out);

    return STATUS_SUCCESS;
}

NTSTATUS fsync_create_mutex( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr, BOOLEAN initial )
{
//    TRACE("name %s, initial %d.\n",
//        attr ? debugstr_us(attr->ObjectName) : "<no name>", initial);

    return create_fsync( FSYNC_MUTEX, handle, access, attr,
        initial ? GetCurrentThreadId() : 0, initial ? 1 : 0 );
}

NTSTATUS fsync_open_mutex( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr )
{
//    TRACE("name %s.\n", debugstr_us(attr->ObjectName));

    return open_fsync( FSYNC_MUTEX, handle, access, attr );
}

NTSTATUS fsync_release_mutex( HANDLE handle, LONG *prev )
{
    struct mutex *mutex;
    struct fsync *obj;
    NTSTATUS ret;

//    TRACE("%p, %p.\n", handle, prev);

    if ((ret = get_object( handle, &obj ))) return ret;
    mutex = obj->shm;

    if (mutex->tid != GetCurrentThreadId()) return STATUS_MUTANT_NOT_OWNED;

    if (prev) *prev = mutex->count;

    if (!--mutex->count)
    {
        __atomic_store_n( &mutex->tid, 0, __ATOMIC_SEQ_CST );
        futex_wake( &mutex->tid, INT_MAX );
    }

    return STATUS_SUCCESS;
}

NTSTATUS fsync_query_mutex( HANDLE handle, MUTANT_INFORMATION_CLASS class,
    void *info, ULONG len, ULONG *ret_len )
{
    struct fsync *obj;
    struct mutex *mutex;
    MUTANT_BASIC_INFORMATION *out = info;
    NTSTATUS ret;

//    TRACE("%p, %u, %p, %u, %p.\n", handle, class, info, len, ret_len);

    if (class != MutantBasicInformation)
    {
        FIXME("(%p,%d,%u) Unknown class\n", handle, class, len);
        return STATUS_INVALID_INFO_CLASS;
    }

    if ((ret = get_object( handle, &obj ))) return ret;
    mutex = obj->shm;

    out->CurrentCount = 1 - mutex->count;
    out->OwnedByCaller = (mutex->tid == GetCurrentThreadId());
    out->AbandonedState = FALSE;
    if (ret_len) *ret_len = sizeof(*out);

    return STATUS_SUCCESS;
}

#define TICKSPERSEC        ((ULONGLONG)10000000)

static NTSTATUS do_single_wait( int *addr, int val, ULONGLONG *end, BOOLEAN alertable )
{
    int ret;

    if (alertable)
    {
        int *apc_futex = ntdll_get_thread_data()->fsync_apc_futex;
        struct futex_waitv futexes[2];

        if (__atomic_load_n( apc_futex, __ATOMIC_SEQ_CST ))
            return STATUS_USER_APC;

        futex_vector_set( &futexes[0], addr, val );
        futex_vector_set( &futexes[1], apc_futex, 0 );

        ret = futex_wait_multiple( futexes, 2, end );

        if (__atomic_load_n( apc_futex, __ATOMIC_SEQ_CST ))
            return STATUS_USER_APC;
    }
    else
    {
        ret = futex_wait( addr, val, end );
    }

    if (!ret)
        return 0;
    else if (ret < 0 && errno == ETIMEDOUT)
        return STATUS_TIMEOUT;
    else
        return STATUS_PENDING;
}

static NTSTATUS __fsync_wait_objects( DWORD count, const HANDLE *handles,
    BOOLEAN wait_any, BOOLEAN alertable, const LARGE_INTEGER *timeout )
{
    static const LARGE_INTEGER zero = {0};

    struct futex_waitv futexes[MAXIMUM_WAIT_OBJECTS + 1];
    struct fsync *objs[MAXIMUM_WAIT_OBJECTS];
    int has_fsync = 0, has_server = 0;
    BOOL msgwait = FALSE;
    int dummy_futex = 0;
    LONGLONG timeleft;
    LARGE_INTEGER now;
    DWORD waitcount;
    ULONGLONG end;
    int i, ret;

    /* Grab the APC futex if we don't already have it. */
    if (alertable && !ntdll_get_thread_data()->fsync_apc_futex)
    {
        unsigned int idx = 0;
        SERVER_START_REQ( get_fsync_apc_idx )
        {
            if (!(ret = wine_server_call( req )))
                idx = reply->shm_idx;
        }
        SERVER_END_REQ;

        if (idx)
        {
            struct event *apc_event = get_shm( idx );
            ntdll_get_thread_data()->fsync_apc_futex = &apc_event->signaled;
        }
    }

    NtQuerySystemTime( &now );
    if (timeout)
    {
        if (timeout->QuadPart == TIMEOUT_INFINITE)
            timeout = NULL;
        else if (timeout->QuadPart > 0)
            end = timeout->QuadPart;
        else
            end = now.QuadPart - timeout->QuadPart;
    }

    for (i = 0; i < count; i++)
    {
        ret = get_object( handles[i], &objs[i] );
        if (ret == STATUS_SUCCESS)
            has_fsync = 1;
        else if (ret == STATUS_NOT_IMPLEMENTED)
            has_server = 1;
        else
            return ret;
    }

    if (objs[count - 1] && objs[count - 1]->type == FSYNC_QUEUE)
        msgwait = TRUE;

    if (has_fsync && has_server)
        FIXME("Can't wait on fsync and server objects at the same time!\n");
    else if (has_server)
        return STATUS_NOT_IMPLEMENTED;




    if (wait_any || count == 1)
    {
        while (1)
        {
            /* Try to grab anything. */

            if (alertable)
            {
                /* We must check this first! The server may set an event that
                 * we're waiting on, but we need to return STATUS_USER_APC. */
                if (__atomic_load_n( ntdll_get_thread_data()->fsync_apc_futex, __ATOMIC_SEQ_CST ))
                    goto userapc;
            }

            for (i = 0; i < count; i++)
            {
                struct fsync *obj = objs[i];

                if (obj)
                {
                    if (!obj->type) /* gcc complains if we put this in the switch */
                    {
                        /* Someone probably closed an object while waiting on it. */
                        WARN("Handle %p has type 0; was it closed?\n", handles[i]);
                        return STATUS_INVALID_HANDLE;
                    }

                    switch (obj->type)
                    {
                    case FSYNC_SEMAPHORE:
                    {
                        struct semaphore *semaphore = obj->shm;
                        int current;

                        do
                        {
                            if (!(current = semaphore->count)) break;
                        } while (__sync_val_compare_and_swap( &semaphore->count, current, current - 1 ) != current);

                        if (current)
                        {
//                            TRACE("Woken up by handle %p [%d].\n", handles[i], i);
                            return i;
                        }

                        futex_vector_set( &futexes[i], &semaphore->count, 0 );
                        break;
                    }
                    case FSYNC_MUTEX:
                    {
                        struct mutex *mutex = obj->shm;
                        int tid;

                        if (mutex->tid == GetCurrentThreadId())
                        {
//                            TRACE("Woken up by handle %p [%d].\n", handles[i], i);
                            mutex->count++;
                            return i;
                        }

                        if (!(tid = __sync_val_compare_and_swap( &mutex->tid, 0, GetCurrentThreadId() )))
                        {
//                            TRACE("Woken up by handle %p [%d].\n", handles[i], i);
                            mutex->count = 1;
                            return i;
                        }

                        futex_vector_set( &futexes[i], &mutex->tid, tid );
                        break;
                    }
                    case FSYNC_AUTO_EVENT:
                    case FSYNC_AUTO_SERVER:
                    {
                        struct event *event = obj->shm;

                        if (__sync_val_compare_and_swap( &event->signaled, 1, 0 ))
                        {
//                            TRACE("Woken up by handle %p [%d].\n", handles[i], i);
                            return i;
                        }

                        futex_vector_set( &futexes[i], &event->signaled, 0 );
                        break;
                    }
                    case FSYNC_MANUAL_EVENT:
                    case FSYNC_MANUAL_SERVER:
                    case FSYNC_QUEUE:
                    {
                        struct event *event = obj->shm;

                        if (__atomic_load_n( &event->signaled, __ATOMIC_SEQ_CST ))
                        {
//                            TRACE("Woken up by handle %p [%d].\n", handles[i], i);
                            return i;
                        }

                        futex_vector_set( &futexes[i], &event->signaled, 0 );
                        break;
                    }
                    default:
                        ERR("Invalid type %#x for handle %p.\n", obj->type, handles[i]);
                        assert(0);
                    }
                }
                else
                {
                    /* Avoid breaking things entirely. */
                    futex_vector_set( &futexes[i], &dummy_futex, dummy_futex );
                }
            }

            if (alertable)
            {
                /* We already checked if it was signaled; don't bother doing it again. */
                futex_vector_set( &futexes[i++], ntdll_get_thread_data()->fsync_apc_futex, 0 );
            }
            waitcount = i;

            /* Looks like everything is contended, so wait. */

            if (timeout && !timeout->QuadPart)
            {
                /* Unlike esync, we already know that we've timed out, so we
                 * can avoid a syscall. */
                TRACE("Wait timed out.\n");
                return STATUS_TIMEOUT;
            }

            if (waitcount == 1)
                ret = futex_wait( u64_to_ptr(futexes[0].uaddr), futexes[0].val, timeout ? &end : NULL );
            else
                ret = futex_wait_multiple( futexes, waitcount, timeout ? &end : NULL );

            /* FUTEX_WAIT_MULTIPLE can succeed or return -EINTR, -EAGAIN,
             * -EFAULT/-EACCES, -ETIMEDOUT. In the first three cases we need to
             * try again, bad address is already handled by the fact that we
             * tried to read from it, so only break out on a timeout. */
            if (ret == -1 && errno == ETIMEDOUT)
            {
                TRACE("Wait timed out.\n");
                return STATUS_TIMEOUT;
            }
        } /* while (1) */
    }
    else
    {
        /* Wait-all is a little trickier to implement correctly. Fortunately,
         * it's not as common.
         *
         * The idea is basically just to wait in sequence on every object in the
         * set. Then when we're done, try to grab them all in a tight loop. If
         * that fails, release any resources we've grabbed (and yes, we can
         * reliably do this—it's just mutexes and semaphores that we have to
         * put back, and in both cases we just put back 1), and if any of that
         * fails we start over.
         *
         * What makes this inherently bad is that we might temporarily grab a
         * resource incorrectly. Hopefully it'll be quick (and hey, it won't
         * block on wineserver) so nobody will notice. Besides, consider: if
         * object A becomes signaled but someone grabs it before we can grab it
         * and everything else, then they could just as well have grabbed it
         * before it became signaled. Similarly if object A was signaled and we
         * were blocking on object B, then B becomes available and someone grabs
         * A before we can, then they might have grabbed A before B became
         * signaled. In either case anyone who tries to wait on A or B will be
         * waiting for an instant while we put things back. */

        NTSTATUS status = STATUS_SUCCESS;
        int current;

        while (1)
        {
tryagain:
            /* First step: try to wait on each object in sequence. */

            for (i = 0; i < count; i++)
            {
                struct fsync *obj = objs[i];

                if (obj && obj->type == FSYNC_MUTEX)
                {
                    struct mutex *mutex = obj->shm;

                    if (mutex->tid == GetCurrentThreadId())
                        continue;

                    while ((current = __atomic_load_n( &mutex->tid, __ATOMIC_SEQ_CST )))
                    {
                        status = do_single_wait( &mutex->tid, current, timeout ? &end : NULL, alertable );
                        if (status != STATUS_PENDING)
                            break;
                    }
                }
                else if (obj)
                {
                    /* this works for semaphores too */
                    struct event *event = obj->shm;

                    while (!__atomic_load_n( &event->signaled, __ATOMIC_SEQ_CST ))
                    {
                        status = do_single_wait( &event->signaled, 0, timeout ? &end : NULL, alertable );
                        if (status != STATUS_PENDING)
                            break;
                    }
                }

                if (status == STATUS_TIMEOUT)
                {
                    TRACE("Wait timed out.\n");
                    return status;
                }
                else if (status == STATUS_USER_APC)
                    goto userapc;
            }

            /* If we got here and we haven't timed out, that means all of the
             * handles were signaled. Check to make sure they still are. */
            for (i = 0; i < count; i++)
            {
                struct fsync *obj = objs[i];

                if (obj && obj->type == FSYNC_MUTEX)
                {
                    struct mutex *mutex = obj->shm;

                    if (mutex->tid == GetCurrentThreadId())
                        continue;   /* ok */

                    if (__atomic_load_n( &mutex->tid, __ATOMIC_SEQ_CST ))
                        goto tryagain;
                }
                else if (obj)
                {
                    struct event *event = obj->shm;

                    if (!__atomic_load_n( &event->signaled, __ATOMIC_SEQ_CST ))
                        goto tryagain;
                }
            }

            /* Yep, still signaled. Now quick, grab everything. */
            for (i = 0; i < count; i++)
            {
                struct fsync *obj = objs[i];
                switch (obj->type)
                {
                case FSYNC_MUTEX:
                {
                    struct mutex *mutex = obj->shm;
                    if (mutex->tid == GetCurrentThreadId())
                        break;
                    if (__sync_val_compare_and_swap( &mutex->tid, 0, GetCurrentThreadId() ))
                        goto tooslow;
                    break;
                }
                case FSYNC_SEMAPHORE:
                {
                    struct semaphore *semaphore = obj->shm;
                    if (__sync_fetch_and_sub( &semaphore->count, 1 ) <= 0)
                        goto tooslow;
                    break;
                }
                case FSYNC_AUTO_EVENT:
                case FSYNC_AUTO_SERVER:
                {
                    struct event *event = obj->shm;
                    if (!__sync_val_compare_and_swap( &event->signaled, 1, 0 ))
                        goto tooslow;
                    break;
                }
                default:
                    /* If a manual-reset event changed between there and
                     * here, it's shouldn't be a problem. */
                    break;
                }
            }

            /* If we got here, we successfully waited on every object.
             * Make sure to let ourselves know that we grabbed the mutexes. */
            for (i = 0; i < count; i++)
            {
                if (objs[i] && objs[i]->type == FSYNC_MUTEX)
                {
                    struct mutex *mutex = objs[i]->shm;
                    mutex->count++;
                }
            }

            TRACE("Wait successful.\n");
            return STATUS_SUCCESS;

tooslow:
            for (--i; i >= 0; i--)
            {
                struct fsync *obj = objs[i];
                switch (obj->type)
                {
                case FSYNC_MUTEX:
                {
                    struct mutex *mutex = obj->shm;
                    __atomic_store_n( &mutex->tid, 0, __ATOMIC_SEQ_CST );
                    break;
                }
                case FSYNC_SEMAPHORE:
                {
                    struct semaphore *semaphore = obj->shm;
                    __sync_fetch_and_add( &semaphore->count, 1 );
                    break;
                }
                case FSYNC_AUTO_EVENT:
                case FSYNC_AUTO_SERVER:
                {
                    struct event *event = obj->shm;
                    __atomic_store_n( &event->signaled, 1, __ATOMIC_SEQ_CST );
                    break;
                }
                default:
                    /* doesn't need to be put back */
                    break;
                }
            }
        } /* while (1) */
    } /* else (wait-all) */

    assert(0);  /* shouldn't reach here... */

userapc:
    TRACE("Woken up by user APC.\n");

    /* We have to make a server call anyway to get the APC to execute, so just
     * delegate down to server_wait(). */
    ret = server_wait( NULL, 0, SELECT_INTERRUPTIBLE | SELECT_ALERTABLE, &zero );

    /* This can happen if we received a system APC, and the APC fd was woken up
     * before we got SIGUSR1. poll() doesn't return EINTR in that case. The
     * right thing to do seems to be to return STATUS_USER_APC anyway. */
    if (ret == STATUS_TIMEOUT) ret = STATUS_USER_APC;
    return ret;
}

/* Like esync, we need to let the server know when we are doing a message wait,
 * and when we are done with one, so that all of the code surrounding hung
 * queues works, and we also need this for WaitForInputIdle().
 *
 * Unlike esync, we can't wait on the queue fd itself locally. Instead we let
 * the server do that for us, the way it normally does. This could actually
 * work for esync too, and that might be better. */
static void server_set_msgwait( int in_msgwait )
{
    SERVER_START_REQ( fsync_msgwait )
    {
        req->in_msgwait = in_msgwait;
        wine_server_call( req );
    }
    SERVER_END_REQ;
}

/* This is a very thin wrapper around the proper implementation above. The
 * purpose is to make sure the server knows when we are doing a message wait.
 * This is separated into a wrapper function since there are at least a dozen
 * exit paths from fsync_wait_objects(). */
NTSTATUS fsync_wait_objects( DWORD count, const HANDLE *handles, BOOLEAN wait_any,
                             BOOLEAN alertable, const LARGE_INTEGER *timeout )
{
    BOOL msgwait = FALSE;
    struct fsync *obj;
    NTSTATUS ret;

    if (!get_object( handles[count - 1], &obj ) && obj->type == FSYNC_QUEUE)
    {
        msgwait = TRUE;
        server_set_msgwait( 1 );
    }

    ret = __fsync_wait_objects( count, handles, wait_any, alertable, timeout );

    if (msgwait)
        server_set_msgwait( 0 );

    return ret;
}

NTSTATUS fsync_signal_and_wait( HANDLE signal, HANDLE wait, BOOLEAN alertable,
    const LARGE_INTEGER *timeout )
{
    struct fsync *obj;
    NTSTATUS ret;

    if ((ret = get_object( signal, &obj ))) return ret;

    switch (obj->type)
    {
    case FSYNC_SEMAPHORE:
        ret = fsync_release_semaphore( signal, 1, NULL );
        break;
    case FSYNC_AUTO_EVENT:
    case FSYNC_MANUAL_EVENT:
        ret = fsync_set_event( signal, NULL );
        break;
    case FSYNC_MUTEX:
        ret = fsync_release_mutex( signal, NULL );
        break;
    default:
        return STATUS_OBJECT_TYPE_MISMATCH;
    }
    if (ret) return ret;

    return fsync_wait_objects( 1, &wait, TRUE, alertable, timeout );
}
