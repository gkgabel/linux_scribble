/* SPDX-License-Identifier: GPL-2.0 */


#ifndef __LINUX_PLATFORM_DATA_CLK_S3C2410_H_
#define __LINUX_PLATFORM_DATA_CLK_S3C2410_H_


struct s3c2410_clk_platform_data {
	unsigned int (*modify_misccr)(unsigned int clr, unsigned int chg);
};

#endif 

