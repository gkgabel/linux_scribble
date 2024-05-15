/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __LINUX_SPI_S3C24XX_H
#define __LINUX_SPI_S3C24XX_H __FILE__

struct s3c2410_spi_info {
	unsigned int		 num_cs;	
	int			 bus_num;       
	unsigned int		 use_fiq:1;	
};

extern int s3c24xx_set_fiq(unsigned int irq, u32 *ack_ptr, bool on);

#endif 
