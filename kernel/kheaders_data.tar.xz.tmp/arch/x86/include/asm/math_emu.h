/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_X86_MATH_EMU_H
#define _ASM_X86_MATH_EMU_H

#include <asm/ptrace.h>


struct math_emu_info {
	long ___orig_eip;
	struct pt_regs *regs;
};
#endif 
