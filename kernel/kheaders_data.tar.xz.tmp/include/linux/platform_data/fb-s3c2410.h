/* SPDX-License-Identifier: GPL-2.0 */


#ifndef __ASM_PLAT_FB_S3C2410_H
#define __ASM_PLAT_FB_S3C2410_H __FILE__

#include <linux/compiler_types.h>

struct s3c2410fb_hw {
	unsigned long	lcdcon1;
	unsigned long	lcdcon2;
	unsigned long	lcdcon3;
	unsigned long	lcdcon4;
	unsigned long	lcdcon5;
};


struct s3c2410fb_display {
	
	unsigned type;
#define S3C2410_LCDCON1_DSCAN4	   (0<<5)
#define S3C2410_LCDCON1_STN4	   (1<<5)
#define S3C2410_LCDCON1_STN8	   (2<<5)
#define S3C2410_LCDCON1_TFT	   (3<<5)

#define S3C2410_LCDCON1_TFT1BPP	   (8<<1)
#define S3C2410_LCDCON1_TFT2BPP	   (9<<1)
#define S3C2410_LCDCON1_TFT4BPP	   (10<<1)
#define S3C2410_LCDCON1_TFT8BPP	   (11<<1)
#define S3C2410_LCDCON1_TFT16BPP   (12<<1)
#define S3C2410_LCDCON1_TFT24BPP   (13<<1)

	
	unsigned short width;
	unsigned short height;

	
	unsigned short xres;
	unsigned short yres;
	unsigned short bpp;

	unsigned pixclock;		
	unsigned short left_margin;  
	unsigned short right_margin; 
	unsigned short hsync_len;    
	unsigned short upper_margin;	
	unsigned short lower_margin;	
	unsigned short vsync_len;	

	
	unsigned long	lcdcon5;
#define S3C2410_LCDCON5_BPP24BL	    (1<<12)
#define S3C2410_LCDCON5_FRM565	    (1<<11)
#define S3C2410_LCDCON5_INVVCLK	    (1<<10)
#define S3C2410_LCDCON5_INVVLINE    (1<<9)
#define S3C2410_LCDCON5_INVVFRAME   (1<<8)
#define S3C2410_LCDCON5_INVVD	    (1<<7)
#define S3C2410_LCDCON5_INVVDEN	    (1<<6)
#define S3C2410_LCDCON5_INVPWREN    (1<<5)
#define S3C2410_LCDCON5_INVLEND	    (1<<4)
#define S3C2410_LCDCON5_PWREN	    (1<<3)
#define S3C2410_LCDCON5_ENLEND	    (1<<2)
#define S3C2410_LCDCON5_BSWP	    (1<<1)
#define S3C2410_LCDCON5_HWSWP	    (1<<0)
};

struct s3c2410fb_mach_info {

	struct s3c2410fb_display *displays;	
	unsigned num_displays;			
	unsigned default_display;

	

	unsigned long	gpcup;
	unsigned long	gpcup_mask;
	unsigned long	gpccon;
	unsigned long	gpccon_mask;
	unsigned long	gpdup;
	unsigned long	gpdup_mask;
	unsigned long	gpdcon;
	unsigned long	gpdcon_mask;

	void __iomem *  gpccon_reg;
	void __iomem *  gpcup_reg;
	void __iomem *  gpdcon_reg;
	void __iomem *  gpdup_reg;

	
	unsigned long	lpcsel;
};

extern void s3c24xx_fb_set_platdata(struct s3c2410fb_mach_info *);

#endif 
