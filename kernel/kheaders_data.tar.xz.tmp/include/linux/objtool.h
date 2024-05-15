/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_OBJTOOL_H
#define _LINUX_OBJTOOL_H

#ifndef __ASSEMBLY__

#include <linux/types.h>


struct unwind_hint {
	u32		ip;
	s16		sp_offset;
	u8		sp_reg;
	u8		type;
	u8		end;
};
#endif


#define UNWIND_HINT_TYPE_CALL		0
#define UNWIND_HINT_TYPE_REGS		1
#define UNWIND_HINT_TYPE_REGS_PARTIAL	2
#define UNWIND_HINT_TYPE_FUNC		3
#define UNWIND_HINT_TYPE_ENTRY		4
#define UNWIND_HINT_TYPE_SAVE		5
#define UNWIND_HINT_TYPE_RESTORE	6

#ifdef CONFIG_OBJTOOL

#include <asm/asm.h>

#ifndef __ASSEMBLY__

#define UNWIND_HINT(sp_reg, sp_offset, type, end)		\
	"987: \n\t"						\
	".pushsection .discard.unwind_hints\n\t"		\
					\
	".long 987b - .\n\t"					\
	".short " __stringify(sp_offset) "\n\t"			\
	".byte " __stringify(sp_reg) "\n\t"			\
	".byte " __stringify(type) "\n\t"			\
	".byte " __stringify(end) "\n\t"			\
	".balign 4 \n\t"					\
	".popsection\n\t"


#define STACK_FRAME_NON_STANDARD(func) \
	static void __used __section(".discard.func_stack_frame_non_standard") \
		*__func_stack_frame_non_standard_##func = func


#ifdef CONFIG_FRAME_POINTER
#define STACK_FRAME_NON_STANDARD_FP(func) STACK_FRAME_NON_STANDARD(func)
#else
#define STACK_FRAME_NON_STANDARD_FP(func)
#endif

#define ANNOTATE_NOENDBR					\
	"986: \n\t"						\
	".pushsection .discard.noendbr\n\t"			\
	_ASM_PTR " 986b\n\t"					\
	".popsection\n\t"

#define ASM_REACHABLE							\
	"998:\n\t"							\
	".pushsection .discard.reachable\n\t"				\
	".long 998b - .\n\t"						\
	".popsection\n\t"

#else 


#define ANNOTATE_INTRA_FUNCTION_CALL				\
	999:							\
	.pushsection .discard.intra_function_calls;		\
	.long 999b;						\
	.popsection;


.macro UNWIND_HINT type:req sp_reg=0 sp_offset=0 end=0
.Lunwind_hint_ip_\@:
	.pushsection .discard.unwind_hints
		
		.long .Lunwind_hint_ip_\@ - .
		.short \sp_offset
		.byte \sp_reg
		.byte \type
		.byte \end
		.balign 4
	.popsection
.endm

.macro STACK_FRAME_NON_STANDARD func:req
	.pushsection .discard.func_stack_frame_non_standard, "aw"
	_ASM_PTR \func
	.popsection
.endm

.macro STACK_FRAME_NON_STANDARD_FP func:req
#ifdef CONFIG_FRAME_POINTER
	STACK_FRAME_NON_STANDARD \func
#endif
.endm

.macro ANNOTATE_NOENDBR
.Lhere_\@:
	.pushsection .discard.noendbr
	.quad	.Lhere_\@
	.popsection
.endm

.macro REACHABLE
.Lhere_\@:
	.pushsection .discard.reachable
	.long	.Lhere_\@ - .
	.popsection
.endm

#endif 

#else 

#ifndef __ASSEMBLY__

#define UNWIND_HINT(sp_reg, sp_offset, type, end)	\
	"\n\t"
#define STACK_FRAME_NON_STANDARD(func)
#define STACK_FRAME_NON_STANDARD_FP(func)
#define ANNOTATE_NOENDBR
#define ASM_REACHABLE
#else
#define ANNOTATE_INTRA_FUNCTION_CALL
.macro UNWIND_HINT type:req sp_reg=0 sp_offset=0 end=0
.endm
.macro STACK_FRAME_NON_STANDARD func:req
.endm
.macro ANNOTATE_NOENDBR
.endm
.macro REACHABLE
.endm
#endif

#endif 

#endif 
