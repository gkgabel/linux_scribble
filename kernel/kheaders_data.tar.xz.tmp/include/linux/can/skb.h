/* SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause) */


#ifndef _CAN_SKB_H
#define _CAN_SKB_H

#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/can.h>
#include <net/sock.h>

void can_flush_echo_skb(struct net_device *dev);
int can_put_echo_skb(struct sk_buff *skb, struct net_device *dev,
		     unsigned int idx, unsigned int frame_len);
struct sk_buff *__can_get_echo_skb(struct net_device *dev, unsigned int idx,
				   u8 *len_ptr, unsigned int *frame_len_ptr);
unsigned int __must_check can_get_echo_skb(struct net_device *dev,
					   unsigned int idx,
					   unsigned int *frame_len_ptr);
void can_free_echo_skb(struct net_device *dev, unsigned int idx,
		       unsigned int *frame_len_ptr);
struct sk_buff *alloc_can_skb(struct net_device *dev, struct can_frame **cf);
struct sk_buff *alloc_canfd_skb(struct net_device *dev,
				struct canfd_frame **cfd);
struct sk_buff *alloc_can_err_skb(struct net_device *dev,
				  struct can_frame **cf);
bool can_dropped_invalid_skb(struct net_device *dev, struct sk_buff *skb);




struct can_skb_priv {
	int ifindex;
	int skbcnt;
	unsigned int frame_len;
	struct can_frame cf[];
};

static inline struct can_skb_priv *can_skb_prv(struct sk_buff *skb)
{
	return (struct can_skb_priv *)(skb->head);
}

static inline void can_skb_reserve(struct sk_buff *skb)
{
	skb_reserve(skb, sizeof(struct can_skb_priv));
}

static inline void can_skb_set_owner(struct sk_buff *skb, struct sock *sk)
{
	
	if (sk && refcount_inc_not_zero(&sk->sk_refcnt)) {
		skb->destructor = sock_efree;
		skb->sk = sk;
	}
}


static inline struct sk_buff *can_create_echo_skb(struct sk_buff *skb)
{
	struct sk_buff *nskb;

	nskb = skb_clone(skb, GFP_ATOMIC);
	if (unlikely(!nskb)) {
		kfree_skb(skb);
		return NULL;
	}

	can_skb_set_owner(nskb, skb->sk);
	consume_skb(skb);
	return nskb;
}

static inline bool can_is_canfd_skb(const struct sk_buff *skb)
{
	
	return skb->len == CANFD_MTU;
}

#endif 
