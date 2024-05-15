/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */

#ifndef _UAPI_LINUX_USER_EVENTS_H
#define _UAPI_LINUX_USER_EVENTS_H

#include <linux/types.h>
#include <linux/ioctl.h>

#ifdef __KERNEL__
#include <linux/uio.h>
#else
#include <sys/uio.h>
#endif

#define USER_EVENTS_SYSTEM "user_events"
#define USER_EVENTS_PREFIX "u:"


#define EVENT_BIT_FTRACE 0
#define EVENT_BIT_PERF 1
#define EVENT_BIT_OTHER 7

#define EVENT_STATUS_FTRACE (1 << EVENT_BIT_FTRACE)
#define EVENT_STATUS_PERF (1 << EVENT_BIT_PERF)
#define EVENT_STATUS_OTHER (1 << EVENT_BIT_OTHER)


#define DYN_LOC(offset, size) ((size) << 16 | (offset))


struct user_reg {

	
	__u32 size;

	
	__u64 name_args;

	
	__u32 status_index;

	
	__u32 write_index;
};

#define DIAG_IOC_MAGIC '*'


#define DIAG_IOCSREG _IOWR(DIAG_IOC_MAGIC, 0, struct user_reg*)


#define DIAG_IOCSDEL _IOW(DIAG_IOC_MAGIC, 1, char*)

#endif 
