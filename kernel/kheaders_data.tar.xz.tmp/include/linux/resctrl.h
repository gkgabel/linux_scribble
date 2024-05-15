/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _RESCTRL_H
#define _RESCTRL_H

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/pid.h>

#ifdef CONFIG_PROC_CPU_RESCTRL

int proc_resctrl_show(struct seq_file *m,
		      struct pid_namespace *ns,
		      struct pid *pid,
		      struct task_struct *tsk);

#endif


enum resctrl_conf_type {
	CDP_NONE,
	CDP_CODE,
	CDP_DATA,
};

#define CDP_NUM_TYPES	(CDP_DATA + 1)


struct resctrl_staged_config {
	u32			new_ctrl;
	bool			have_new_ctrl;
};


struct rdt_domain {
	struct list_head		list;
	int				id;
	struct cpumask			cpu_mask;
	unsigned long			*rmid_busy_llc;
	struct mbm_state		*mbm_total;
	struct mbm_state		*mbm_local;
	struct delayed_work		mbm_over;
	struct delayed_work		cqm_limbo;
	int				mbm_work_cpu;
	int				cqm_work_cpu;
	struct pseudo_lock_region	*plr;
	struct resctrl_staged_config	staged_config[CDP_NUM_TYPES];
};


struct resctrl_cache {
	unsigned int	cbm_len;
	unsigned int	min_cbm_bits;
	unsigned int	shareable_bits;
	bool		arch_has_sparse_bitmaps;
	bool		arch_has_empty_bitmaps;
	bool		arch_has_per_cpu_cfg;
};


enum membw_throttle_mode {
	THREAD_THROTTLE_UNDEFINED = 0,
	THREAD_THROTTLE_MAX,
	THREAD_THROTTLE_PER_THREAD,
};


struct resctrl_membw {
	u32				min_bw;
	u32				bw_gran;
	u32				delay_linear;
	bool				arch_needs_linear;
	enum membw_throttle_mode	throttle_mode;
	bool				mba_sc;
	u32				*mb_map;
};

struct rdt_parse_data;
struct resctrl_schema;


struct rdt_resource {
	int			rid;
	bool			alloc_enabled;
	bool			mon_enabled;
	bool			alloc_capable;
	bool			mon_capable;
	int			num_rmid;
	int			cache_level;
	struct resctrl_cache	cache;
	struct resctrl_membw	membw;
	struct list_head	domains;
	char			*name;
	int			data_width;
	u32			default_ctrl;
	const char		*format_str;
	int			(*parse_ctrlval)(struct rdt_parse_data *data,
						 struct resctrl_schema *s,
						 struct rdt_domain *d);
	struct list_head	evt_list;
	unsigned long		fflags;
	bool			cdp_capable;
};


struct resctrl_schema {
	struct list_head		list;
	char				name[8];
	enum resctrl_conf_type		conf_type;
	struct rdt_resource		*res;
	u32				num_closid;
};


u32 resctrl_arch_get_num_closid(struct rdt_resource *r);
int resctrl_arch_update_domains(struct rdt_resource *r, u32 closid);
u32 resctrl_arch_get_config(struct rdt_resource *r, struct rdt_domain *d,
			    u32 closid, enum resctrl_conf_type type);

#endif 
