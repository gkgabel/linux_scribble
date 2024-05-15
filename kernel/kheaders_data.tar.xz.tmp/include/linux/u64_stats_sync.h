/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_U64_STATS_SYNC_H
#define _LINUX_U64_STATS_SYNC_H


#include <linux/seqlock.h>

struct u64_stats_sync {
#if BITS_PER_LONG == 32 && (defined(CONFIG_SMP) || defined(CONFIG_PREEMPT_RT))
	seqcount_t	seq;
#endif
};

#if BITS_PER_LONG == 64
#include <asm/local64.h>

typedef struct {
	local64_t	v;
} u64_stats_t ;

static inline u64 u64_stats_read(const u64_stats_t *p)
{
	return local64_read(&p->v);
}

static inline void u64_stats_set(u64_stats_t *p, u64 val)
{
	local64_set(&p->v, val);
}

static inline void u64_stats_add(u64_stats_t *p, unsigned long val)
{
	local64_add(val, &p->v);
}

static inline void u64_stats_inc(u64_stats_t *p)
{
	local64_inc(&p->v);
}

#else

typedef struct {
	u64		v;
} u64_stats_t;

static inline u64 u64_stats_read(const u64_stats_t *p)
{
	return p->v;
}

static inline void u64_stats_set(u64_stats_t *p, u64 val)
{
	p->v = val;
}

static inline void u64_stats_add(u64_stats_t *p, unsigned long val)
{
	p->v += val;
}

static inline void u64_stats_inc(u64_stats_t *p)
{
	p->v++;
}
#endif

#if BITS_PER_LONG == 32 && (defined(CONFIG_SMP) || defined(CONFIG_PREEMPT_RT))
#define u64_stats_init(syncp)	seqcount_init(&(syncp)->seq)
#else
static inline void u64_stats_init(struct u64_stats_sync *syncp)
{
}
#endif

static inline void u64_stats_update_begin(struct u64_stats_sync *syncp)
{
#if BITS_PER_LONG == 32 && (defined(CONFIG_SMP) || defined(CONFIG_PREEMPT_RT))
	if (IS_ENABLED(CONFIG_PREEMPT_RT))
		preempt_disable();
	write_seqcount_begin(&syncp->seq);
#endif
}

static inline void u64_stats_update_end(struct u64_stats_sync *syncp)
{
#if BITS_PER_LONG == 32 && (defined(CONFIG_SMP) || defined(CONFIG_PREEMPT_RT))
	write_seqcount_end(&syncp->seq);
	if (IS_ENABLED(CONFIG_PREEMPT_RT))
		preempt_enable();
#endif
}

static inline unsigned long
u64_stats_update_begin_irqsave(struct u64_stats_sync *syncp)
{
	unsigned long flags = 0;

#if BITS_PER_LONG == 32 && (defined(CONFIG_SMP) || defined(CONFIG_PREEMPT_RT))
	if (IS_ENABLED(CONFIG_PREEMPT_RT))
		preempt_disable();
	else
		local_irq_save(flags);
	write_seqcount_begin(&syncp->seq);
#endif
	return flags;
}

static inline void
u64_stats_update_end_irqrestore(struct u64_stats_sync *syncp,
				unsigned long flags)
{
#if BITS_PER_LONG == 32 && (defined(CONFIG_SMP) || defined(CONFIG_PREEMPT_RT))
	write_seqcount_end(&syncp->seq);
	if (IS_ENABLED(CONFIG_PREEMPT_RT))
		preempt_enable();
	else
		local_irq_restore(flags);
#endif
}

static inline unsigned int __u64_stats_fetch_begin(const struct u64_stats_sync *syncp)
{
#if BITS_PER_LONG == 32 && (defined(CONFIG_SMP) || defined(CONFIG_PREEMPT_RT))
	return read_seqcount_begin(&syncp->seq);
#else
	return 0;
#endif
}

static inline unsigned int u64_stats_fetch_begin(const struct u64_stats_sync *syncp)
{
#if BITS_PER_LONG == 32 && (!defined(CONFIG_SMP) && !defined(CONFIG_PREEMPT_RT))
	preempt_disable();
#endif
	return __u64_stats_fetch_begin(syncp);
}

static inline bool __u64_stats_fetch_retry(const struct u64_stats_sync *syncp,
					 unsigned int start)
{
#if BITS_PER_LONG == 32 && (defined(CONFIG_SMP) || defined(CONFIG_PREEMPT_RT))
	return read_seqcount_retry(&syncp->seq, start);
#else
	return false;
#endif
}

static inline bool u64_stats_fetch_retry(const struct u64_stats_sync *syncp,
					 unsigned int start)
{
#if BITS_PER_LONG == 32 && (!defined(CONFIG_SMP) && !defined(CONFIG_PREEMPT_RT))
	preempt_enable();
#endif
	return __u64_stats_fetch_retry(syncp, start);
}


static inline unsigned int u64_stats_fetch_begin_irq(const struct u64_stats_sync *syncp)
{
#if BITS_PER_LONG == 32 && defined(CONFIG_PREEMPT_RT)
	preempt_disable();
#elif BITS_PER_LONG == 32 && !defined(CONFIG_SMP)
	local_irq_disable();
#endif
	return __u64_stats_fetch_begin(syncp);
}

static inline bool u64_stats_fetch_retry_irq(const struct u64_stats_sync *syncp,
					     unsigned int start)
{
#if BITS_PER_LONG == 32 && defined(CONFIG_PREEMPT_RT)
	preempt_enable();
#elif BITS_PER_LONG == 32 && !defined(CONFIG_SMP)
	local_irq_enable();
#endif
	return __u64_stats_fetch_retry(syncp, start);
}

#endif 
