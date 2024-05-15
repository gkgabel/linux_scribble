/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef GENERIC_ADC_BATTERY_H
#define GENERIC_ADC_BATTERY_H


struct gab_platform_data {
	struct power_supply_info battery_info;
	int	(*cal_charge)(long value);
	int     jitter_delay;
};

#endif 
