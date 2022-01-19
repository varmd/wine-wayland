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

#if 0
#pragma makedep unix
#endif

#include "config.h"

#include <assert.h>
#include <errno.h>
#ifdef HAVE_POLL_H
#include <poll.h>
#endif
#ifdef HAVE_SYS_POLL_H
# include <sys/poll.h>
#endif
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ntstatus.h"
#define WIN32_NO_STATUS
#define NONAMELESSUNION
#include "windef.h"
#include "winternl.h"
#include "wine/server.h"
#include "wine/debug.h"
//#include "wine/library.h"

#include "unix_private.h"
#include "esync.h"


WINE_DEFAULT_DEBUG_CHANNEL(esync);


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


static int global_esync_active = 0;

static pthread_mutex_t shm_addrs_section = PTHREAD_MUTEX_INITIALIZER;;

void activate_esync(void) {
  global_esync_active = 1;
}

int do_esync(void)
{
  //Esync depreciated
  //Only Fsync
  return 0;

}

/* Entry point for drivers to set queue fd. */
extern void CDECL esync_set_queue_fd( int fd )
{
    TRACE("Setting Esync queue fd \n");
    ntdll_get_thread_data()->esync_queue_fd = fd;
}

struct esync
{
    enum esync_type type;   /* defined in protocol.def */
    int fd;
    void *shm;              /* pointer to shm section */
};

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

static char shm_name[29];
static int shm_fd;
static void **shm_addrs;
static int shm_addrs_size;  /* length of the allocated shm_addrs array */
static long pagesize;

static NTSTATUS create_esync( enum esync_type type, HANDLE *handle,
    ACCESS_MASK access, const OBJECT_ATTRIBUTES *attr, int initval, int max );

void esync_init(void)
{


    TRACE("Esync loaded \n");

}

static void *get_shm( unsigned int idx )
{

}

/* We'd like lookup to be fast. To that end, we use a static list indexed by handle.
 * This is copied and adapted from the fd cache code. */

#define ESYNC_LIST_BLOCK_SIZE  (65536 / sizeof(struct esync))
#define ESYNC_LIST_ENTRIES     256

static struct esync *esync_list[ESYNC_LIST_ENTRIES];
static struct esync esync_list_initial_block[ESYNC_LIST_BLOCK_SIZE];

static inline UINT_PTR handle_to_index( HANDLE handle, UINT_PTR *entry )
{
    UINT_PTR idx = (((UINT_PTR)handle) >> 2) - 1;
    *entry = idx / ESYNC_LIST_BLOCK_SIZE;
    return idx % ESYNC_LIST_BLOCK_SIZE;
}

static struct esync *add_to_list( HANDLE handle, enum esync_type type, int fd, void *shm )
{
    UINT_PTR entry, idx = handle_to_index( handle, &entry );

    if (entry >= ESYNC_LIST_ENTRIES)
    {
        FIXME( "too many allocated handles, not caching %p\n", handle );
        return FALSE;
    }

    if (!esync_list[entry])  /* do we need to allocate a new block of entries? */
    {
        if (!entry) esync_list[0] = esync_list_initial_block;
        else
        {
            void *ptr = wine_anon_mmap( NULL, ESYNC_LIST_BLOCK_SIZE * sizeof(struct esync),
                                        PROT_READ | PROT_WRITE, 0 );
            if (ptr == MAP_FAILED) return FALSE;
            esync_list[entry] = ptr;
        }
    }

    if (!InterlockedCompareExchange((int *)&esync_list[entry][idx].type, type, 0))
    {
        esync_list[entry][idx].fd = fd;
        esync_list[entry][idx].shm = shm;
    }
    return &esync_list[entry][idx];
}

static struct esync *get_cached_object( HANDLE handle )
{
    UINT_PTR entry, idx = handle_to_index( handle, &entry );

    if (entry >= ESYNC_LIST_ENTRIES || !esync_list[entry]) return NULL;
    if (!esync_list[entry][idx].type) return NULL;

    return &esync_list[entry][idx];
}

/* Gets an object. This is either a proper esync object (i.e. an event,
 * semaphore, etc. created using create_esync) or a generic synchronizable
 * server-side object which the server will signal (e.g. a process, thread,
 * message queue, etc.)
 *
 * Note that we have to make the server path available even for esync objects
 * since we might be passed a duplicated or inherited handle. */
static NTSTATUS get_object( HANDLE handle, struct esync **obj )
{
    NTSTATUS ret = STATUS_SUCCESS;

    return ret;
}

NTSTATUS esync_close( HANDLE handle )
{
    UINT_PTR entry, idx = handle_to_index( handle, &entry );

    //TRACE("%p %d \n", handle, &entry );

    if (entry < ESYNC_LIST_ENTRIES && esync_list[entry])
    {
        if (InterlockedExchange((int *)&esync_list[entry][idx].type, 0))
        {
            close( esync_list[entry][idx].fd );
            //printf("Realy closing %d \n", ret_close);
            return STATUS_SUCCESS;
        }
    }

    //printf("Invalid handle %d \n", &entry);

    return STATUS_INVALID_HANDLE;
}

static NTSTATUS create_esync( enum esync_type type, HANDLE *handle,
    ACCESS_MASK access, const OBJECT_ATTRIBUTES *attr, int initval, int max )
{
    NTSTATUS ret;
    data_size_t len;
    struct object_attributes *objattr;
    obj_handle_t fd_handle;
    unsigned int shm_idx;
    sigset_t sigset;
    int fd;

    if ((ret = alloc_object_attributes( attr, &objattr, &len ))) return ret;

    /* We have to synchronize on the fd cache CS so that our calls to
     * receive_fd don't race with theirs. */
    server_enter_uninterrupted_section( &fd_cache_mutex, &sigset );
    SERVER_START_REQ( create_esync )
    {
        req->access  = access;
        req->initval = initval;
        req->type    = type;
        req->max     = max;
        wine_server_add_data( req, objattr, len );
        ret = wine_server_call( req );
        if (!ret || ret == STATUS_OBJECT_NAME_EXISTS)
        {
            *handle = wine_server_ptr_handle( reply->handle );
            type = reply->type;
            shm_idx = reply->shm_idx;
            fd = receive_fd( &fd_handle );
            assert( wine_server_ptr_handle(fd_handle) == *handle );
        }
    }
    SERVER_END_REQ;
    server_leave_uninterrupted_section( &fd_cache_mutex, &sigset );

    if (!ret || ret == STATUS_OBJECT_NAME_EXISTS)
    {
        add_to_list( *handle, type, fd, shm_idx ? get_shm( shm_idx ) : 0 );

        TRACE("-> handle %p, fd %d, shm index %d.\n", *handle, fd, shm_idx);
    }

    free( objattr );
    return ret;
}

static NTSTATUS open_esync( enum esync_type type, HANDLE *handle,
    ACCESS_MASK access, const OBJECT_ATTRIBUTES *attr )
{
    NTSTATUS ret;

    return ret;
}

NTSTATUS esync_create_semaphore(HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr, LONG initial, LONG max)
{
    TRACE("name %s, initial %d, max %d.\n",
        attr ? debugstr_us(attr->ObjectName) : "<no name>", initial, max);

    return create_esync( ESYNC_SEMAPHORE, handle, access, attr, initial, max );
}

NTSTATUS esync_open_semaphore( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr )
{
    TRACE("name %s.\n", debugstr_us(attr->ObjectName));

    return open_esync( ESYNC_SEMAPHORE, handle, access, attr );
}

NTSTATUS esync_release_semaphore( HANDLE handle, ULONG count, ULONG *prev )
{
    
    return STATUS_SUCCESS;
}

NTSTATUS esync_query_semaphore( HANDLE handle, SEMAPHORE_INFORMATION_CLASS class,
    void *info, ULONG len, ULONG *ret_len )
{
    struct esync *obj;
    struct semaphore *semaphore;
    SEMAPHORE_BASIC_INFORMATION *out = info;
    NTSTATUS ret;

    TRACE("%p, %u, %p, %u, %p.\n", handle, class, info, len, ret_len);

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

NTSTATUS esync_create_event( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr, EVENT_TYPE event_type, BOOLEAN initial )
{
    enum esync_type type = (event_type == SynchronizationEvent ? ESYNC_AUTO_EVENT : ESYNC_MANUAL_EVENT);

    TRACE("name %s, %s-reset, initial %d.\n",
        attr ? debugstr_us(attr->ObjectName) : "<no name>",
        event_type == NotificationEvent ? "manual" : "auto", initial);

    return create_esync( type, handle, access, attr, initial, 0 );
}

NTSTATUS esync_open_event( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr )
{
    TRACE("name %s.\n", debugstr_us(attr->ObjectName));

    return open_esync( ESYNC_AUTO_EVENT, handle, access, attr ); /* doesn't matter which */
}

static inline void small_pause(void)
{
#ifdef __i386__
    __asm__ __volatile__( "rep;nop" : : : "memory" );
#else
    __asm__ __volatile__( "" : : : "memory" );
#endif
}

/* Manual-reset events are actually racier than other objects in terms of shm
 * state. With other objects, races don't matter, because we only treat the shm
 * state as a hint that lets us skip poll()—we still have to read(). But with
 * manual-reset events we don't, which means that the shm state can be out of
 * sync with the actual state.
 *
 * In general we shouldn't have to worry about races between modifying the
 * event and waiting on it. If the state changes while we're waiting, it's
 * equally plausible that we caught it before or after the state changed.
 * However, we can have races between SetEvent() and ResetEvent(), so that the
 * event has inconsistent internal state.
 *
 * To solve this we have to use the other field to lock the event. Currently
 * this is implemented as a spinlock, but I'm not sure if a futex might be
 * better. I'm also not sure if it's possible to obviate locking by arranging
 * writes and reads in a certain way.
 *
 * Note that we don't have to worry about locking in esync_wait_objects().
 * There's only two general patterns:
 *
 * WaitFor()    SetEvent()
 * -------------------------
 * read()
 * signaled = 0
 *              signaled = 1
 *              write()
 * -------------------------
 * read()
 *              signaled = 1
 * signaled = 0
 *              <no write(), because it was already signaled>
 * -------------------------
 *
 * That is, if SetEvent() tries to signal the event before WaitFor() resets its
 * signaled state, it won't bother trying to write(), and then the signaled
 * state will be reset, so the result is a consistent non-signaled event.
 * There's several variations to this pattern but all of them are protected in
 * the same way. Note however this is why we have to use interlocked_xchg()
 * event inside of the lock.
 *
 * And of course if SetEvent() follows WaitFor() entirely, well, there's no
 * problem at all.
 */

NTSTATUS esync_set_event( HANDLE handle )
{
    static const uint64_t value = 1;
    struct esync *obj;
    struct event *event;
    NTSTATUS ret;

    TRACE("%p.\n", handle);

    if ((ret = get_object( handle, &obj ))) return ret;
    event = obj->shm;

    /* Acquire the spinlock. */
    while (InterlockedCompareExchange( &event->locked, 1, 0 ))
        small_pause();

    /* Only bother signaling the fd if we weren't already signaled. */
    if (!InterlockedExchange( &event->signaled, 1 ))
    {
        if (write( obj->fd, &value, sizeof(value) ) == -1)
            return errno_to_status( errno );;
    }

    /* Release the spinlock. */
    event->locked = 0;

    return STATUS_SUCCESS;
}

NTSTATUS esync_reset_event( HANDLE handle )
{
    uint64_t value;
    struct esync *obj;
    struct event *event;
    NTSTATUS ret;

    TRACE("%p.\n", handle);

    if ((ret = get_object( handle, &obj ))) return ret;
    event = obj->shm;

    /* Acquire the spinlock. */
    while (InterlockedCompareExchange( &event->locked, 1, 0 ))
        small_pause();

    /* Only bother signaling the fd if we weren't already signaled. */
    if (InterlockedExchange( &event->signaled, 0 ))
    {
        /* we don't care about the return value */
        read( obj->fd, &value, sizeof(value) );
    }

    /* Release the spinlock. */
    event->locked = 0;

    return STATUS_SUCCESS;
}

NTSTATUS esync_pulse_event( HANDLE handle )
{
    uint64_t value = 1;
    struct esync *obj;
    NTSTATUS ret;

    TRACE("%p.\n", handle);

    if ((ret = get_object( handle, &obj ))) return ret;

    /* This isn't really correct; an application could miss the write.
     * Unfortunately we can't really do much better. Fortunately this is rarely
     * used (and publicly deprecated). */
    if (write( obj->fd, &value, sizeof(value) ) == -1)
        return errno_to_status( errno );;

    /* Try to give other threads a chance to wake up. Hopefully erring on this
     * side is the better thing to do... */
    NtYieldExecution();

    read( obj->fd, &value, sizeof(value) );

    return STATUS_SUCCESS;
}

NTSTATUS esync_query_event( HANDLE handle, EVENT_INFORMATION_CLASS class,
    void *info, ULONG len, ULONG *ret_len )
{


    return STATUS_SUCCESS;
}

NTSTATUS esync_create_mutex( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr, BOOLEAN initial )
{
    TRACE("name %s, initial %d.\n",
        attr ? debugstr_us(attr->ObjectName) : "<no name>", initial);

    return create_esync( ESYNC_MUTEX, handle, access, attr, initial ? 0 : 1, 0 );
}

NTSTATUS esync_open_mutex( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr )
{
    TRACE("name %s.\n", debugstr_us(attr->ObjectName));

    return open_esync( ESYNC_MUTEX, handle, access, attr );
}

NTSTATUS esync_release_mutex( HANDLE *handle, LONG *prev )
{
    struct esync *obj;
    struct mutex *mutex;
    static const uint64_t value = 1;
    NTSTATUS ret;

    TRACE("%p, %p.\n", handle, prev);

    if ((ret = get_object( handle, &obj ))) return ret;
    mutex = obj->shm;

    /* This is thread-safe, because the only thread that can change the tid to
     * or from our tid is ours. */
    if (mutex->tid != GetCurrentThreadId()) return STATUS_MUTANT_NOT_OWNED;

    if (prev) *prev = mutex->count;

    mutex->count--;

    if (!mutex->count)
    {
        /* This is also thread-safe, as long as signaling the file is the last
         * thing we do. Other threads don't care about the tid if it isn't
         * theirs. */
        mutex->tid = 0;

        if (write( obj->fd, &value, sizeof(value) ) == -1)
            return errno_to_status( errno );;
    }

    return STATUS_SUCCESS;
}

NTSTATUS esync_query_mutex( HANDLE handle, MUTANT_INFORMATION_CLASS class,
    void *info, ULONG len, ULONG *ret_len )
{
    struct esync *obj;
    struct mutex *mutex;
    MUTANT_BASIC_INFORMATION *out = info;
    NTSTATUS ret;

    TRACE("%p, %u, %p, %u, %p.\n", handle, class, info, len, ret_len);

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

#define TICKSPERSEC        10000000
#define TICKSPERMSEC       10000

static LONGLONG update_timeout( ULONGLONG end )
{
    LARGE_INTEGER now;
    LONGLONG timeleft;

    NtQuerySystemTime( &now );
    timeleft = end - now.QuadPart;
    if (timeleft < 0) timeleft = 0;
    return timeleft;
}

static int do_poll(  )
{
    int ret = 0;



    return ret;
}

static void update_grabbed_object( struct esync *obj )
{

}

/* A value of STATUS_NOT_IMPLEMENTED returned from this function means that we
 * need to delegate to server_select(). */
static NTSTATUS __esync_wait_objects( DWORD count, const HANDLE *handles,
    BOOLEAN wait_any, BOOLEAN alertable, const LARGE_INTEGER *timeout )
{
    //Do nothing esync deprecated


    int ret;


    return ret;
}

/* We need to let the server know when we are doing a message wait, and when we
 * are done with one, so that all of the code surrounding hung queues works.
 * We also need this for WaitForInputIdle(). */
static void server_set_msgwait( int in_msgwait )
{
    SERVER_START_REQ( esync_msgwait )
    {
        req->in_msgwait = in_msgwait;
        wine_server_call( req );
    }
    SERVER_END_REQ;
}

/* This is a very thin wrapper around the proper implementation above. The
 * purpose is to make sure the server knows when we are doing a message wait.
 * This is separated into a wrapper function since there are at least a dozen
 * exit paths from esync_wait_objects(). */
NTSTATUS esync_wait_objects( DWORD count, const HANDLE *handles, BOOLEAN wait_any,
                             BOOLEAN alertable, const LARGE_INTEGER *timeout )
{

    NTSTATUS ret;



    return ret;
}

NTSTATUS esync_signal_and_wait( HANDLE signal, HANDLE wait, BOOLEAN alertable,
    const LARGE_INTEGER *timeout )
{
    struct esync *obj;
    NTSTATUS ret;

    if ((ret = get_object( signal, &obj ))) return ret;

    switch (obj->type)
    {
    case ESYNC_SEMAPHORE:
        ret = esync_release_semaphore( signal, 1, NULL );
        break;
    case ESYNC_AUTO_EVENT:
    case ESYNC_MANUAL_EVENT:
        ret = esync_set_event( signal );
        break;
    case ESYNC_MUTEX:
        ret = esync_release_mutex( signal, NULL );
        break;
    default:
        return STATUS_OBJECT_TYPE_MISMATCH;
    }
    if (ret) return ret;

    return esync_wait_objects( 1, &wait, TRUE, alertable, timeout );
}
