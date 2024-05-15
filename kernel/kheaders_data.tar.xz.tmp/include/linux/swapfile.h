/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SWAPFILE_H
#define _LINUX_SWAPFILE_H


extern struct swap_info_struct *swap_info[];
extern unsigned long generic_max_swapfile_size(void);
extern unsigned long max_swapfile_size(void);

#endif 
