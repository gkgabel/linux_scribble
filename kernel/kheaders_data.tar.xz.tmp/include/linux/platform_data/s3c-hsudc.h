/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __LINUX_USB_S3C_HSUDC_H
#define __LINUX_USB_S3C_HSUDC_H


struct s3c24xx_hsudc_platdata {
	unsigned int	epnum;
	void		(*gpio_init)(void);
	void		(*gpio_uninit)(void);
	void		(*phy_init)(void);
	void		(*phy_uninit)(void);
};

#endif	
