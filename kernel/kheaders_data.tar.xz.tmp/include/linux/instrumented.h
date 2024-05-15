/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _LINUX_INSTRUMENTED_H
#define _LINUX_INSTRUMENTED_H

#include <linux/compiler.h>
#include <linux/kasan-checks.h>
#include <linux/kcsan-checks.h>
#include <linux/types.h>


static __always_inline void instrument_read(const volatile void *v, size_t size)
{
	kasan_check_read(v, size);
	kcsan_check_read(v, size);
}


static __always_inline void instrument_write(const volatile void *v, size_t size)
{
	kasan_check_write(v, size);
	kcsan_check_write(v, size);
}


static __always_inline void instrument_read_write(const volatile void *v, size_t size)
{
	kasan_check_write(v, size);
	kcsan_check_read_write(v, size);
}


static __always_inline void instrument_atomic_read(const volatile void *v, size_t size)
{
	kasan_check_read(v, size);
	kcsan_check_atomic_read(v, size);
}


static __always_inline void instrument_atomic_write(const volatile void *v, size_t size)
{
	kasan_check_write(v, size);
	kcsan_check_atomic_write(v, size);
}


static __always_inline void instrument_atomic_read_write(const volatile void *v, size_t size)
{
	kasan_check_write(v, size);
	kcsan_check_atomic_read_write(v, size);
}


static __always_inline void
instrument_copy_to_user(void __user *to, const void *from, unsigned long n)
{
	kasan_check_read(from, n);
	kcsan_check_read(from, n);
}


static __always_inline void
instrument_copy_from_user(const void *to, const void __user *from, unsigned long n)
{
	kasan_check_write(to, n);
	kcsan_check_write(to, n);
}

#endif 
