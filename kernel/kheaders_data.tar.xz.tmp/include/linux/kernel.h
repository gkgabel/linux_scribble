/* SPDX-License-Identifier: GPL-2.0 */

#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H

#include <linux/stdarg.h>
#include <linux/align.h>
#include <linux/limits.h>
#include <linux/linkage.h>
#include <linux/stddef.h>
#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/container_of.h>
#include <linux/bitops.h>
#include <linux/kstrtox.h>
#include <linux/log2.h>
#include <linux/math.h>
#include <linux/minmax.h>
#include <linux/typecheck.h>
#include <linux/panic.h>
#include <linux/printk.h>
#include <linux/build_bug.h>
#include <linux/static_call_types.h>
#include <linux/instruction_pointer.h>
#include <asm/byteorder.h>

#include <uapi/linux/kernel.h>

#define STACK_MAGIC	0xdeadbeef


#define REPEAT_BYTE(x)	((~0ul / 0xff) * (x))


#define READ			0
#define WRITE			1


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

#define PTR_IF(cond, ptr)	((cond) ? (ptr) : NULL)

#define u64_to_user_ptr(x) (		\
{					\
	typecheck(u64, (x));		\
	(void __user *)(uintptr_t)(x);	\
}					\
)


#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))


#define lower_32_bits(n) ((u32)((n) & 0xffffffff))


#define upper_16_bits(n) ((u16)((n) >> 16))


#define lower_16_bits(n) ((u16)((n) & 0xffff))

struct completion;
struct user;

#ifdef CONFIG_PREEMPT_VOLUNTARY_BUILD

extern int __cond_resched(void);
# define might_resched() __cond_resched()

#elif defined(CONFIG_PREEMPT_DYNAMIC) && defined(CONFIG_HAVE_PREEMPT_DYNAMIC_CALL)

extern int __cond_resched(void);

DECLARE_STATIC_CALL(might_resched, __cond_resched);

static __always_inline void might_resched(void)
{
	static_call_mod(might_resched)();
}

#elif defined(CONFIG_PREEMPT_DYNAMIC) && defined(CONFIG_HAVE_PREEMPT_DYNAMIC_KEY)

extern int dynamic_might_resched(void);
# define might_resched() dynamic_might_resched()

#else

# define might_resched() do { } while (0)

#endif 

#ifdef CONFIG_DEBUG_ATOMIC_SLEEP
extern void __might_resched(const char *file, int line, unsigned int offsets);
extern void __might_sleep(const char *file, int line);
extern void __cant_sleep(const char *file, int line, int preempt_offset);
extern void __cant_migrate(const char *file, int line);


# define might_sleep() \
	do { __might_sleep(__FILE__, __LINE__); might_resched(); } while (0)

# define cant_sleep() \
	do { __cant_sleep(__FILE__, __LINE__, 0); } while (0)
# define sched_annotate_sleep()	(current->task_state_change = 0)


# define cant_migrate()							\
	do {								\
		if (IS_ENABLED(CONFIG_SMP))				\
			__cant_migrate(__FILE__, __LINE__);		\
	} while (0)


# define non_block_start() (current->non_block_count++)

# define non_block_end() WARN_ON(current->non_block_count-- == 0)
#else
  static inline void __might_resched(const char *file, int line,
				     unsigned int offsets) { }
static inline void __might_sleep(const char *file, int line) { }
# define might_sleep() do { might_resched(); } while (0)
# define cant_sleep() do { } while (0)
# define cant_migrate()		do { } while (0)
# define sched_annotate_sleep() do { } while (0)
# define non_block_start() do { } while (0)
# define non_block_end() do { } while (0)
#endif

#define might_sleep_if(cond) do { if (cond) might_sleep(); } while (0)

#if defined(CONFIG_MMU) && \
	(defined(CONFIG_PROVE_LOCKING) || defined(CONFIG_DEBUG_ATOMIC_SLEEP))
#define might_fault() __might_fault(__FILE__, __LINE__)
void __might_fault(const char *file, int line);
#else
static inline void might_fault(void) { }
#endif

void do_exit(long error_code) __noreturn;

extern int num_to_str(char *buf, int size,
		      unsigned long long num, unsigned int width);



extern __printf(2, 3) int sprintf(char *buf, const char * fmt, ...);
extern __printf(2, 0) int vsprintf(char *buf, const char *, va_list);
extern __printf(3, 4)
int snprintf(char *buf, size_t size, const char *fmt, ...);
extern __printf(3, 0)
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
extern __printf(3, 4)
int scnprintf(char *buf, size_t size, const char *fmt, ...);
extern __printf(3, 0)
int vscnprintf(char *buf, size_t size, const char *fmt, va_list args);
extern __printf(2, 3) __malloc
char *kasprintf(gfp_t gfp, const char *fmt, ...);
extern __printf(2, 0) __malloc
char *kvasprintf(gfp_t gfp, const char *fmt, va_list args);
extern __printf(2, 0)
const char *kvasprintf_const(gfp_t gfp, const char *fmt, va_list args);

extern __scanf(2, 3)
int sscanf(const char *, const char *, ...);
extern __scanf(2, 0)
int vsscanf(const char *, const char *, va_list);

extern int no_hash_pointers_enable(char *str);

extern int get_option(char **str, int *pint);
extern char *get_options(const char *str, int nints, int *ints);
extern unsigned long long memparse(const char *ptr, char **retptr);
extern bool parse_option_str(const char *str, const char *option);
extern char *next_arg(char *args, char **param, char **val);

extern int core_kernel_text(unsigned long addr);
extern int __kernel_text_address(unsigned long addr);
extern int kernel_text_address(unsigned long addr);
extern int func_ptr_is_kernel_text(void *ptr);

extern void bust_spinlocks(int yes);

extern int root_mountflags;

extern bool early_boot_irqs_disabled;


extern enum system_states {
	SYSTEM_BOOTING,
	SYSTEM_SCHEDULING,
	SYSTEM_FREEING_INITMEM,
	SYSTEM_RUNNING,
	SYSTEM_HALT,
	SYSTEM_POWER_OFF,
	SYSTEM_RESTART,
	SYSTEM_SUSPEND,
} system_state;

extern const char hex_asc[];
#define hex_asc_lo(x)	hex_asc[((x) & 0x0f)]
#define hex_asc_hi(x)	hex_asc[((x) & 0xf0) >> 4]

static inline char *hex_byte_pack(char *buf, u8 byte)
{
	*buf++ = hex_asc_hi(byte);
	*buf++ = hex_asc_lo(byte);
	return buf;
}

extern const char hex_asc_upper[];
#define hex_asc_upper_lo(x)	hex_asc_upper[((x) & 0x0f)]
#define hex_asc_upper_hi(x)	hex_asc_upper[((x) & 0xf0) >> 4]

static inline char *hex_byte_pack_upper(char *buf, u8 byte)
{
	*buf++ = hex_asc_upper_hi(byte);
	*buf++ = hex_asc_upper_lo(byte);
	return buf;
}

extern int hex_to_bin(unsigned char ch);
extern int __must_check hex2bin(u8 *dst, const char *src, size_t count);
extern char *bin2hex(char *dst, const void *src, size_t count);

bool mac_pton(const char *s, u8 *mac);



enum ftrace_dump_mode {
	DUMP_NONE,
	DUMP_ALL,
	DUMP_ORIG,
};

#ifdef CONFIG_TRACING
void tracing_on(void);
void tracing_off(void);
int tracing_is_on(void);
void tracing_snapshot(void);
void tracing_snapshot_alloc(void);

extern void tracing_start(void);
extern void tracing_stop(void);

static inline __printf(1, 2)
void ____trace_printk_check_format(const char *fmt, ...)
{
}
#define __trace_printk_check_format(fmt, args...)			\
do {									\
	if (0)								\
		____trace_printk_check_format(fmt, ##args);		\
} while (0)



#define trace_printk(fmt, ...)				\
do {							\
	char _______STR[] = __stringify((__VA_ARGS__));	\
	if (sizeof(_______STR) > 3)			\
		do_trace_printk(fmt, ##__VA_ARGS__);	\
	else						\
		trace_puts(fmt);			\
} while (0)

#define do_trace_printk(fmt, args...)					\
do {									\
	static const char *trace_printk_fmt __used			\
		__section("__trace_printk_fmt") =			\
		__builtin_constant_p(fmt) ? fmt : NULL;			\
									\
	__trace_printk_check_format(fmt, ##args);			\
									\
	if (__builtin_constant_p(fmt))					\
		__trace_bprintk(_THIS_IP_, trace_printk_fmt, ##args);	\
	else								\
		__trace_printk(_THIS_IP_, fmt, ##args);			\
} while (0)

extern __printf(2, 3)
int __trace_bprintk(unsigned long ip, const char *fmt, ...);

extern __printf(2, 3)
int __trace_printk(unsigned long ip, const char *fmt, ...);



#define trace_puts(str) ({						\
	static const char *trace_printk_fmt __used			\
		__section("__trace_printk_fmt") =			\
		__builtin_constant_p(str) ? str : NULL;			\
									\
	if (__builtin_constant_p(str))					\
		__trace_bputs(_THIS_IP_, trace_printk_fmt);		\
	else								\
		__trace_puts(_THIS_IP_, str, strlen(str));		\
})
extern int __trace_bputs(unsigned long ip, const char *str);
extern int __trace_puts(unsigned long ip, const char *str, int size);

extern void trace_dump_stack(int skip);


#define ftrace_vprintk(fmt, vargs)					\
do {									\
	if (__builtin_constant_p(fmt)) {				\
		static const char *trace_printk_fmt __used		\
		  __section("__trace_printk_fmt") =			\
			__builtin_constant_p(fmt) ? fmt : NULL;		\
									\
		__ftrace_vbprintk(_THIS_IP_, trace_printk_fmt, vargs);	\
	} else								\
		__ftrace_vprintk(_THIS_IP_, fmt, vargs);		\
} while (0)

extern __printf(2, 0) int
__ftrace_vbprintk(unsigned long ip, const char *fmt, va_list ap);

extern __printf(2, 0) int
__ftrace_vprintk(unsigned long ip, const char *fmt, va_list ap);

extern void ftrace_dump(enum ftrace_dump_mode oops_dump_mode);
#else
static inline void tracing_start(void) { }
static inline void tracing_stop(void) { }
static inline void trace_dump_stack(int skip) { }

static inline void tracing_on(void) { }
static inline void tracing_off(void) { }
static inline int tracing_is_on(void) { return 0; }
static inline void tracing_snapshot(void) { }
static inline void tracing_snapshot_alloc(void) { }

static inline __printf(1, 2)
int trace_printk(const char *fmt, ...)
{
	return 0;
}
static __printf(1, 0) inline int
ftrace_vprintk(const char *fmt, va_list ap)
{
	return 0;
}
static inline void ftrace_dump(enum ftrace_dump_mode oops_dump_mode) { }
#endif 


#define __COUNT_ARGS(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _n, X...) _n
#define COUNT_ARGS(X...) __COUNT_ARGS(, ##X, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define __CONCAT(a, b) a ## b
#define CONCATENATE(a, b) __CONCAT(a, b)


#ifdef CONFIG_FTRACE_MCOUNT_RECORD
# define REBUILD_DUE_TO_FTRACE_MCOUNT_RECORD
#endif


#define VERIFY_OCTAL_PERMISSIONS(perms)						\
	(BUILD_BUG_ON_ZERO((perms) < 0) +					\
	 BUILD_BUG_ON_ZERO((perms) > 0777) +					\
	 		\
	 BUILD_BUG_ON_ZERO((((perms) >> 6) & 4) < (((perms) >> 3) & 4)) +	\
	 BUILD_BUG_ON_ZERO((((perms) >> 3) & 4) < ((perms) & 4)) +		\
	 					\
	 BUILD_BUG_ON_ZERO((((perms) >> 6) & 2) < (((perms) >> 3) & 2)) +	\
	 		\
	 BUILD_BUG_ON_ZERO((perms) & 2) +					\
	 (perms))
#endif
