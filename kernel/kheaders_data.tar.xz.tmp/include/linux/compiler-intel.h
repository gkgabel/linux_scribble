/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_COMPILER_TYPES_H
#error "Please don't include <linux/compiler-intel.h> directly, include <linux/compiler.h> instead."
#endif

#ifdef __ECC



#include <asm/intrinsics.h>



#define barrier() __memory_barrier()
#define barrier_data(ptr) barrier()

#define RELOC_HIDE(ptr, off)					\
  ({ unsigned long __ptr;					\
     __ptr = (unsigned long) (ptr);				\
    (typeof(ptr)) (__ptr + (off)); })


#define OPTIMIZER_HIDE_VAR(var) barrier()

#endif


#define __HAVE_BUILTIN_BSWAP16__
#define __builtin_bswap16 _bswap16
