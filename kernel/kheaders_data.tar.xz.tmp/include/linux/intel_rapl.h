/* SPDX-License-Identifier: GPL-2.0 */


#ifndef __INTEL_RAPL_H__
#define __INTEL_RAPL_H__

#include <linux/types.h>
#include <linux/powercap.h>
#include <linux/cpuhotplug.h>

enum rapl_domain_type {
	RAPL_DOMAIN_PACKAGE,	
	RAPL_DOMAIN_PP0,	
	RAPL_DOMAIN_PP1,	
	RAPL_DOMAIN_DRAM,	
	RAPL_DOMAIN_PLATFORM,	
	RAPL_DOMAIN_MAX,
};

enum rapl_domain_reg_id {
	RAPL_DOMAIN_REG_LIMIT,
	RAPL_DOMAIN_REG_STATUS,
	RAPL_DOMAIN_REG_PERF,
	RAPL_DOMAIN_REG_POLICY,
	RAPL_DOMAIN_REG_INFO,
	RAPL_DOMAIN_REG_PL4,
	RAPL_DOMAIN_REG_MAX,
};

struct rapl_domain;

enum rapl_primitives {
	ENERGY_COUNTER,
	POWER_LIMIT1,
	POWER_LIMIT2,
	POWER_LIMIT4,
	FW_LOCK,

	PL1_ENABLE,		
	PL1_CLAMP,		
	PL2_ENABLE,		
	PL2_CLAMP,
	PL4_ENABLE,		

	TIME_WINDOW1,		
	TIME_WINDOW2,		
	THERMAL_SPEC_POWER,
	MAX_POWER,

	MIN_POWER,
	MAX_TIME_WINDOW,
	THROTTLED_TIME,
	PRIORITY_LEVEL,

	PSYS_POWER_LIMIT1,
	PSYS_POWER_LIMIT2,
	PSYS_PL1_ENABLE,
	PSYS_PL2_ENABLE,
	PSYS_TIME_WINDOW1,
	PSYS_TIME_WINDOW2,
	
	AVERAGE_POWER,
	NR_RAPL_PRIMITIVES,
};

struct rapl_domain_data {
	u64 primitives[NR_RAPL_PRIMITIVES];
	unsigned long timestamp;
};

#define NR_POWER_LIMITS (3)
struct rapl_power_limit {
	struct powercap_zone_constraint *constraint;
	int prim_id;		
	struct rapl_domain *domain;
	const char *name;
	u64 last_power_limit;
};

struct rapl_package;

#define RAPL_DOMAIN_NAME_LENGTH 16

struct rapl_domain {
	char name[RAPL_DOMAIN_NAME_LENGTH];
	enum rapl_domain_type id;
	u64 regs[RAPL_DOMAIN_REG_MAX];
	struct powercap_zone power_zone;
	struct rapl_domain_data rdd;
	struct rapl_power_limit rpl[NR_POWER_LIMITS];
	u64 attr_map;		
	unsigned int state;
	unsigned int domain_energy_unit;
	struct rapl_package *rp;
};

struct reg_action {
	u64 reg;
	u64 mask;
	u64 value;
	int err;
};


struct rapl_if_priv {
	struct powercap_control_type *control_type;
	struct rapl_domain *platform_rapl_domain;
	enum cpuhp_state pcap_rapl_online;
	u64 reg_unit;
	u64 regs[RAPL_DOMAIN_MAX][RAPL_DOMAIN_REG_MAX];
	int limits[RAPL_DOMAIN_MAX];
	int (*read_raw)(int cpu, struct reg_action *ra);
	int (*write_raw)(int cpu, struct reg_action *ra);
};


#define PACKAGE_DOMAIN_NAME_LENGTH 30

struct rapl_package {
	unsigned int id;	
	unsigned int nr_domains;
	unsigned long domain_map;	
	unsigned int power_unit;
	unsigned int energy_unit;
	unsigned int time_unit;
	struct rapl_domain *domains;	
	struct powercap_zone *power_zone;	
	unsigned long power_limit_irq;	
	struct list_head plist;
	int lead_cpu;		
	
	struct cpumask cpumask;
	char name[PACKAGE_DOMAIN_NAME_LENGTH];
	struct rapl_if_priv *priv;
};

struct rapl_package *rapl_find_package_domain(int cpu, struct rapl_if_priv *priv);
struct rapl_package *rapl_add_package(int cpu, struct rapl_if_priv *priv);
void rapl_remove_package(struct rapl_package *rp);

#endif 
