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

extern void activate_fsync(void);
extern int do_fsync(void);
extern void fsync_init(void);
extern NTSTATUS fsync_close( HANDLE handle );

extern NTSTATUS fsync_create_semaphore(HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr, LONG initial, LONG max);
extern NTSTATUS fsync_release_semaphore( HANDLE handle, ULONG count, ULONG *prev );
extern NTSTATUS fsync_open_semaphore( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr );
extern NTSTATUS fsync_query_semaphore( HANDLE handle, SEMAPHORE_INFORMATION_CLASS class,
    void *info, ULONG len, ULONG *ret_len );
extern NTSTATUS fsync_create_event( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr, EVENT_TYPE type, BOOLEAN initial );
extern NTSTATUS fsync_open_event( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr );
extern NTSTATUS fsync_set_event( HANDLE handle, LONG *prev );
extern NTSTATUS fsync_reset_event( HANDLE handle, LONG *prev );
extern NTSTATUS fsync_pulse_event( HANDLE handle, LONG *prev );
extern NTSTATUS fsync_query_event( HANDLE handle, EVENT_INFORMATION_CLASS class,
    void *info, ULONG len, ULONG *ret_len );
extern NTSTATUS fsync_create_mutex( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr, BOOLEAN initial );
extern NTSTATUS fsync_open_mutex( HANDLE *handle, ACCESS_MASK access,
    const OBJECT_ATTRIBUTES *attr );
extern NTSTATUS fsync_release_mutex( HANDLE handle, LONG *prev );
extern NTSTATUS fsync_query_mutex( HANDLE handle, MUTANT_INFORMATION_CLASS class,
    void *info, ULONG len, ULONG *ret_len );

extern NTSTATUS fsync_wait_objects( DWORD count, const HANDLE *handles, BOOLEAN wait_any,
                                    BOOLEAN alertable, const LARGE_INTEGER *timeout );
extern NTSTATUS fsync_signal_and_wait( HANDLE signal, HANDLE wait,
    BOOLEAN alertable, const LARGE_INTEGER *timeout );
