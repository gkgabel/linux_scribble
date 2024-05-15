/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _LINUX_IP_H
#define _LINUX_IP_H

#include <linux/skbuff.h>
#include <uapi/linux/ip.h>

static inline struct iphdr *ip_hdr(const struct sk_buff *skb)
{
	return (struct iphdr *)skb_network_header(skb);
}

static inline struct iphdr *inner_ip_hdr(const struct sk_buff *skb)
{
	return (struct iphdr *)skb_inner_network_header(skb);
}

static inline struct iphdr *ipip_hdr(const struct sk_buff *skb)
{
	return (struct iphdr *)skb_transport_header(skb);
}

static inline unsigned int ip_transport_len(const struct sk_buff *skb)
{
	return ntohs(ip_hdr(skb)->tot_len) - skb_network_header_len(skb);
}
#endif	
