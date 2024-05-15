/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _LINUX_CFI_H
#define _LINUX_CFI_H

#ifdef CONFIG_CFI_CLANG
typedef void (*cfi_check_fn)(uint64_t id, void *ptr, void *diag);


extern void __cfi_check(uint64_t id, void *ptr, void *diag);


#define __CFI_ADDRESSABLE(fn, __attr) \
	const void *__cfi_jt_ ## fn __visible __attr = (void *)&fn

#ifdef CONFIG_CFI_CLANG_SHADOW

extern void cfi_module_add(struct module *mod, unsigned long base_addr);
extern void cfi_module_remove(struct module *mod, unsigned long base_addr);

#else

static inline void cfi_module_add(struct module *mod, unsigned long base_addr) {}
static inline void cfi_module_remove(struct module *mod, unsigned long base_addr) {}

#endif 

#else 

#ifdef CONFIG_X86_KERNEL_IBT

#define __CFI_ADDRESSABLE(fn, __attr) \
	const void *__cfi_jt_ ## fn __visible __attr = (void *)&fn

#endif 

#endif 

#ifndef __CFI_ADDRESSABLE
#define __CFI_ADDRESSABLE(fn, __attr)
#endif

#endif 
