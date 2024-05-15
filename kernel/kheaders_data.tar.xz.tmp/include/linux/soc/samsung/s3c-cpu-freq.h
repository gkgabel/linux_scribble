/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __LINUX_SOC_SAMSUNG_S3C_CPU_FREQ_H
#define __LINUX_SOC_SAMSUNG_S3C_CPU_FREQ_H

#include <linux/cpufreq.h>

struct s3c_cpufreq_info;
struct s3c_cpufreq_board;
struct s3c_iotimings;


struct s3c_freq {
	unsigned long	fclk;
	unsigned long	armclk;
	unsigned long	hclk_tns;	
	unsigned long	hclk;
	unsigned long	pclk;
};


struct s3c_cpufreq_freqs {
	struct cpufreq_freqs	freqs;
	struct s3c_freq		old;
	struct s3c_freq		new;

	unsigned int		pll_changing:1;
};

#define to_s3c_cpufreq(_cf) container_of(_cf, struct s3c_cpufreq_freqs, freqs)


struct s3c_clkdivs {
	int		p_divisor;
	int		h_divisor;
	int		arm_divisor;
	unsigned char	dvs;
};

#define PLLVAL(_m, _p, _s) (((_m) << 12) | ((_p) << 4) | (_s))


struct s3c_pllval {
	unsigned long		freq;
	unsigned long		pll_reg;
};


struct s3c_cpufreq_board {
	unsigned int	refresh;
	unsigned int	auto_io:1;	
	unsigned int	need_io:1;	

	
	struct s3c_freq	max;	
};


#ifdef CONFIG_ARM_S3C_CPUFREQ
#define __init_or_cpufreq
#else
#define __init_or_cpufreq __init
#endif



#ifdef CONFIG_ARM_S3C_CPUFREQ
extern int s3c_cpufreq_setboard(struct s3c_cpufreq_board *board);
#else

static inline int s3c_cpufreq_setboard(struct s3c_cpufreq_board *board)
{
	return 0;
}
#endif  

#endif
