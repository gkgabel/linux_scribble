/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _CAN_LENGTH_H
#define _CAN_LENGTH_H


#define CAN_FRAME_OVERHEAD_SFF DIV_ROUND_UP(47, 8)


#define CAN_FRAME_OVERHEAD_EFF DIV_ROUND_UP(67, 8)


#define CANFD_FRAME_OVERHEAD_SFF DIV_ROUND_UP(61, 8)


#define CANFD_FRAME_OVERHEAD_EFF DIV_ROUND_UP(80, 8)


#define CAN_FRAME_LEN_MAX (CAN_FRAME_OVERHEAD_EFF + CAN_MAX_DLEN)


#define CANFD_FRAME_LEN_MAX (CANFD_FRAME_OVERHEAD_EFF + CANFD_MAX_DLEN)


#define can_cc_dlc2len(dlc)	(min_t(u8, (dlc), CAN_MAX_DLEN))


static inline u8 can_get_cc_dlc(const struct can_frame *cf, const u32 ctrlmode)
{
	
	if ((ctrlmode & CAN_CTRLMODE_CC_LEN8_DLC) &&
	    (cf->len == CAN_MAX_DLEN) &&
	    (cf->len8_dlc > CAN_MAX_DLEN && cf->len8_dlc <= CAN_MAX_RAW_DLC))
		return cf->len8_dlc;

	
	return cf->len;
}


static inline void can_frame_set_cc_len(struct can_frame *cf, const u8 dlc,
					const u32 ctrlmode)
{
	
	if (ctrlmode & CAN_CTRLMODE_CC_LEN8_DLC && dlc > CAN_MAX_DLEN)
		cf->len8_dlc = dlc;

	
	cf->len = can_cc_dlc2len(dlc);
}


u8 can_fd_dlc2len(u8 dlc);


u8 can_fd_len2dlc(u8 len);


unsigned int can_skb_get_frame_len(const struct sk_buff *skb);


static inline u8 canfd_sanitize_len(u8 len)
{
	return can_fd_dlc2len(can_fd_len2dlc(len));
}

#endif 
