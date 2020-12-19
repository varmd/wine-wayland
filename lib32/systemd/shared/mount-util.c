/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <errno.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include "alloc-util.h"
#include "extract-word.h"
#include "fd-util.h"
#include "fileio.h"
#include "fs-util.h"
#include "hashmap.h"
#include "libmount-util.h"
#include "mount-util.h"
#include "mountpoint-util.h"
#include "parse-util.h"
#include "path-util.h"
#include "set.h"
#include "stat-util.h"
#include "stdio-util.h"
#include "string-util.h"
#include "strv.h"




int mount_fd(const char *source,
             int target_fd,
             const char *filesystemtype,
             unsigned long mountflags,
             const void *data) {


        return 0;
}

int mount_nofollow(
                const char *source,
                const char *target,
                const char *filesystemtype,
                unsigned long mountflags,
                const void *data) {

       return 0;
}

int umount_recursive(const char *prefix, int flags) {
        return 0;
}

static int get_mount_flags(
                struct libmnt_table *table,
                const char *path,
                unsigned long *ret) {

        return 0;
}

/* Use this function only if you do not have direct access to /proc/self/mountinfo but the caller can open it
 * for you. This is the case when /proc is masked or not mounted. Otherwise, use bind_remount_recursive. */
int bind_remount_recursive_with_mountinfo(
                const char *prefix,
                unsigned long new_flags,
                unsigned long flags_mask,
                char **deny_list,
                FILE *proc_self_mountinfo) {

        return 0;
}

int bind_remount_recursive(
                const char *prefix,
                unsigned long new_flags,
                unsigned long flags_mask,
                char **deny_list) {

        return 0;
}

int bind_remount_one_with_mountinfo(
                const char *path,
                unsigned long new_flags,
                unsigned long flags_mask,
                FILE *proc_self_mountinfo) {

        

        return 0;
}

int mount_move_root(const char *path) {
        

        return 0;
}

int repeat_unmount(const char *path, int flags) {
        return 0;

        
}

int mode_to_inaccessible_node(
                const char *runtime_dir,
                mode_t mode,
                char **ret) {

        return 0;
}


int mount_verbose_full(
                int error_log_level,
                const char *what,
                const char *where,
                const char *type,
                unsigned long flags,
                const char *options,
                bool follow_symlink) {

       
        return 0;
}

int umount_verbose(
                int error_log_level,
                const char *what,
                int flags) {

        
        return 0;
}

int mount_option_mangle(
                const char *options,
                unsigned long mount_flags,
                unsigned long *ret_mount_flags,
                char **ret_remaining_options) {

        

        return 0;
}