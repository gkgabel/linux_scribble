/* SPDX-License-Identifier: GPL-2.0 */



#ifndef __CLKSOURCE_HYPERV_TIMER_H
#define __CLKSOURCE_HYPERV_TIMER_H

#include <linux/clocksource.h>
#include <linux/math64.h>
#include <asm/mshyperv.h>

#define HV_MAX_MAX_DELTA_TICKS 0xffffffff
#define HV_MIN_DELTA_TICKS 1

#ifdef CONFIG_HYPERV_TIMER


extern int hv_stimer_alloc(bool have_percpu_irqs);
extern int hv_stimer_cleanup(unsigned int cpu);
extern void hv_stimer_legacy_init(unsigned int cpu, int sint);
extern void hv_stimer_legacy_cleanup(unsigned int cpu);
extern void hv_stimer_global_cleanup(void);
extern void hv_stimer0_isr(void);

extern void hv_init_clocksource(void);

extern struct ms_hyperv_tsc_page *hv_get_tsc_page(void);

static inline notrace u64
hv_read_tsc_page_tsc(const struct ms_hyperv_tsc_page *tsc_pg, u64 *cur_tsc)
{
	u64 scale, offset;
	u32 sequence;

	
	do {
		sequence = READ_ONCE(tsc_pg->tsc_sequence);
		if (!sequence)
			return U64_MAX;
		
		smp_rmb();

		scale = READ_ONCE(tsc_pg->tsc_scale);
		offset = READ_ONCE(tsc_pg->tsc_offset);
		*cur_tsc = hv_get_raw_timer();

		
		smp_rmb();

	} while (READ_ONCE(tsc_pg->tsc_sequence) != sequence);

	return mul_u64_u64_shr(*cur_tsc, scale, 64) + offset;
}

static inline notrace u64
hv_read_tsc_page(const struct ms_hyperv_tsc_page *tsc_pg)
{
	u64 cur_tsc;

	return hv_read_tsc_page_tsc(tsc_pg, &cur_tsc);
}

#else 
static inline struct ms_hyperv_tsc_page *hv_get_tsc_page(void)
{
	return NULL;
}

static inline u64 hv_read_tsc_page_tsc(const struct ms_hyperv_tsc_page *tsc_pg,
				       u64 *cur_tsc)
{
	return U64_MAX;
}

static inline int hv_stimer_cleanup(unsigned int cpu) { return 0; }
static inline void hv_stimer_legacy_init(unsigned int cpu, int sint) {}
static inline void hv_stimer_legacy_cleanup(unsigned int cpu) {}
static inline void hv_stimer_global_cleanup(void) {}
static inline void hv_stimer0_isr(void) {}

#endif 

#endif
