/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _SOCK_H
#define _SOCK_H

#include <linux/hardirq.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/list_nulls.h>
#include <linux/timer.h>
#include <linux/cache.h>
#include <linux/bitops.h>
#include <linux/lockdep.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>	
#include <linux/mm.h>
#include <linux/security.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/page_counter.h>
#include <linux/memcontrol.h>
#include <linux/static_key.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/cgroup-defs.h>
#include <linux/rbtree.h>
#include <linux/rculist_nulls.h>
#include <linux/poll.h>
#include <linux/sockptr.h>
#include <linux/indirect_call_wrapper.h>
#include <linux/atomic.h>
#include <linux/refcount.h>
#include <linux/llist.h>
#include <net/dst.h>
#include <net/checksum.h>
#include <net/tcp_states.h>
#include <linux/net_tstamp.h>
#include <net/l3mdev.h>
#include <uapi/linux/socket.h>




#define SOCK_DEBUGGING
#ifdef SOCK_DEBUGGING
#define SOCK_DEBUG(sk, msg...) do { if ((sk) && sock_flag((sk), SOCK_DBG)) \
					printk(KERN_DEBUG msg); } while (0)
#else

static inline __printf(2, 3)
void SOCK_DEBUG(const struct sock *sk, const char *msg, ...)
{
}
#endif


typedef struct {
	spinlock_t		slock;
	int			owned;
	wait_queue_head_t	wq;
	
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map dep_map;
#endif
} socket_lock_t;

struct sock;
struct proto;
struct net;

typedef __u32 __bitwise __portpair;
typedef __u64 __bitwise __addrpair;


struct sock_common {
	union {
		__addrpair	skc_addrpair;
		struct {
			__be32	skc_daddr;
			__be32	skc_rcv_saddr;
		};
	};
	union  {
		unsigned int	skc_hash;
		__u16		skc_u16hashes[2];
	};
	
	union {
		__portpair	skc_portpair;
		struct {
			__be16	skc_dport;
			__u16	skc_num;
		};
	};

	unsigned short		skc_family;
	volatile unsigned char	skc_state;
	unsigned char		skc_reuse:4;
	unsigned char		skc_reuseport:1;
	unsigned char		skc_ipv6only:1;
	unsigned char		skc_net_refcnt:1;
	int			skc_bound_dev_if;
	union {
		struct hlist_node	skc_bind_node;
		struct hlist_node	skc_portaddr_node;
	};
	struct proto		*skc_prot;
	possible_net_t		skc_net;

#if IS_ENABLED(CONFIG_IPV6)
	struct in6_addr		skc_v6_daddr;
	struct in6_addr		skc_v6_rcv_saddr;
#endif

	atomic64_t		skc_cookie;

	
	union {
		unsigned long	skc_flags;
		struct sock	*skc_listener; 
		struct inet_timewait_death_row *skc_tw_dr; 
	};
	
	
	int			skc_dontcopy_begin[0];
	
	union {
		struct hlist_node	skc_node;
		struct hlist_nulls_node skc_nulls_node;
	};
	unsigned short		skc_tx_queue_mapping;
#ifdef CONFIG_SOCK_RX_QUEUE_MAPPING
	unsigned short		skc_rx_queue_mapping;
#endif
	union {
		int		skc_incoming_cpu;
		u32		skc_rcv_wnd;
		u32		skc_tw_rcv_nxt; 
	};

	refcount_t		skc_refcnt;
	
	int                     skc_dontcopy_end[0];
	union {
		u32		skc_rxhash;
		u32		skc_window_clamp;
		u32		skc_tw_snd_nxt; 
	};
	
};

struct bpf_local_storage;
struct sk_filter;


struct sock {
	
	struct sock_common	__sk_common;
#define sk_node			__sk_common.skc_node
#define sk_nulls_node		__sk_common.skc_nulls_node
#define sk_refcnt		__sk_common.skc_refcnt
#define sk_tx_queue_mapping	__sk_common.skc_tx_queue_mapping
#ifdef CONFIG_SOCK_RX_QUEUE_MAPPING
#define sk_rx_queue_mapping	__sk_common.skc_rx_queue_mapping
#endif

#define sk_dontcopy_begin	__sk_common.skc_dontcopy_begin
#define sk_dontcopy_end		__sk_common.skc_dontcopy_end
#define sk_hash			__sk_common.skc_hash
#define sk_portpair		__sk_common.skc_portpair
#define sk_num			__sk_common.skc_num
#define sk_dport		__sk_common.skc_dport
#define sk_addrpair		__sk_common.skc_addrpair
#define sk_daddr		__sk_common.skc_daddr
#define sk_rcv_saddr		__sk_common.skc_rcv_saddr
#define sk_family		__sk_common.skc_family
#define sk_state		__sk_common.skc_state
#define sk_reuse		__sk_common.skc_reuse
#define sk_reuseport		__sk_common.skc_reuseport
#define sk_ipv6only		__sk_common.skc_ipv6only
#define sk_net_refcnt		__sk_common.skc_net_refcnt
#define sk_bound_dev_if		__sk_common.skc_bound_dev_if
#define sk_bind_node		__sk_common.skc_bind_node
#define sk_prot			__sk_common.skc_prot
#define sk_net			__sk_common.skc_net
#define sk_v6_daddr		__sk_common.skc_v6_daddr
#define sk_v6_rcv_saddr	__sk_common.skc_v6_rcv_saddr
#define sk_cookie		__sk_common.skc_cookie
#define sk_incoming_cpu		__sk_common.skc_incoming_cpu
#define sk_flags		__sk_common.skc_flags
#define sk_rxhash		__sk_common.skc_rxhash

	
	struct dst_entry __rcu	*sk_rx_dst;
	int			sk_rx_dst_ifindex;
	u32			sk_rx_dst_cookie;

	socket_lock_t		sk_lock;
	atomic_t		sk_drops;
	int			sk_rcvlowat;
	struct sk_buff_head	sk_error_queue;
	struct sk_buff_head	sk_receive_queue;
	
	struct {
		atomic_t	rmem_alloc;
		int		len;
		struct sk_buff	*head;
		struct sk_buff	*tail;
	} sk_backlog;

#define sk_rmem_alloc sk_backlog.rmem_alloc

	int			sk_forward_alloc;
	u32			sk_reserved_mem;
#ifdef CONFIG_NET_RX_BUSY_POLL
	unsigned int		sk_ll_usec;
	
	unsigned int		sk_napi_id;
#endif
	int			sk_rcvbuf;

	struct sk_filter __rcu	*sk_filter;
	union {
		struct socket_wq __rcu	*sk_wq;
		
		struct socket_wq	*sk_wq_raw;
		
	};
#ifdef CONFIG_XFRM
	struct xfrm_policy __rcu *sk_policy[2];
#endif

	struct dst_entry __rcu	*sk_dst_cache;
	atomic_t		sk_omem_alloc;
	int			sk_sndbuf;

	
	int			sk_wmem_queued;
	refcount_t		sk_wmem_alloc;
	unsigned long		sk_tsq_flags;
	union {
		struct sk_buff	*sk_send_head;
		struct rb_root	tcp_rtx_queue;
	};
	struct sk_buff_head	sk_write_queue;
	__s32			sk_peek_off;
	int			sk_write_pending;
	__u32			sk_dst_pending_confirm;
	u32			sk_pacing_status; 
	long			sk_sndtimeo;
	struct timer_list	sk_timer;
	__u32			sk_priority;
	__u32			sk_mark;
	unsigned long		sk_pacing_rate; 
	unsigned long		sk_max_pacing_rate;
	struct page_frag	sk_frag;
	netdev_features_t	sk_route_caps;
	int			sk_gso_type;
	unsigned int		sk_gso_max_size;
	gfp_t			sk_allocation;
	__u32			sk_txhash;

	
	u8			sk_gso_disabled : 1,
				sk_kern_sock : 1,
				sk_no_check_tx : 1,
				sk_no_check_rx : 1,
				sk_userlocks : 4;
	u8			sk_pacing_shift;
	u16			sk_type;
	u16			sk_protocol;
	u16			sk_gso_max_segs;
	unsigned long	        sk_lingertime;
	struct proto		*sk_prot_creator;
	rwlock_t		sk_callback_lock;
	int			sk_err,
				sk_err_soft;
	u32			sk_ack_backlog;
	u32			sk_max_ack_backlog;
	kuid_t			sk_uid;
	u8			sk_txrehash;
#ifdef CONFIG_NET_RX_BUSY_POLL
	u8			sk_prefer_busy_poll;
	u16			sk_busy_poll_budget;
#endif
	spinlock_t		sk_peer_lock;
	int			sk_bind_phc;
	struct pid		*sk_peer_pid;
	const struct cred	*sk_peer_cred;

	long			sk_rcvtimeo;
	ktime_t			sk_stamp;
#if BITS_PER_LONG==32
	seqlock_t		sk_stamp_seq;
#endif
	u16			sk_tsflags;
	u8			sk_shutdown;
	atomic_t		sk_tskey;
	atomic_t		sk_zckey;

	u8			sk_clockid;
	u8			sk_txtime_deadline_mode : 1,
				sk_txtime_report_errors : 1,
				sk_txtime_unused : 6;

	struct socket		*sk_socket;
	void			*sk_user_data;
#ifdef CONFIG_SECURITY
	void			*sk_security;
#endif
	struct sock_cgroup_data	sk_cgrp_data;
	struct mem_cgroup	*sk_memcg;
	void			(*sk_state_change)(struct sock *sk);
	void			(*sk_data_ready)(struct sock *sk);
	void			(*sk_write_space)(struct sock *sk);
	void			(*sk_error_report)(struct sock *sk);
	int			(*sk_backlog_rcv)(struct sock *sk,
						  struct sk_buff *skb);
#ifdef CONFIG_SOCK_VALIDATE_XMIT
	struct sk_buff*		(*sk_validate_xmit_skb)(struct sock *sk,
							struct net_device *dev,
							struct sk_buff *skb);
#endif
	void                    (*sk_destruct)(struct sock *sk);
	struct sock_reuseport __rcu	*sk_reuseport_cb;
#ifdef CONFIG_BPF_SYSCALL
	struct bpf_local_storage __rcu	*sk_bpf_storage;
#endif
	struct rcu_head		sk_rcu;
	netns_tracker		ns_tracker;
};

enum sk_pacing {
	SK_PACING_NONE		= 0,
	SK_PACING_NEEDED	= 1,
	SK_PACING_FQ		= 2,
};


#define SK_USER_DATA_NOCOPY	1UL
#define SK_USER_DATA_BPF	2UL
#define SK_USER_DATA_PSOCK	4UL
#define SK_USER_DATA_PTRMASK	~(SK_USER_DATA_NOCOPY | SK_USER_DATA_BPF |\
				  SK_USER_DATA_PSOCK)


static inline bool sk_user_data_is_nocopy(const struct sock *sk)
{
	return ((uintptr_t)sk->sk_user_data & SK_USER_DATA_NOCOPY);
}

#define __sk_user_data(sk) ((*((void __rcu **)&(sk)->sk_user_data)))


static inline void *
__locked_read_sk_user_data_with_flags(const struct sock *sk,
				      uintptr_t flags)
{
	uintptr_t sk_user_data =
		(uintptr_t)rcu_dereference_check(__sk_user_data(sk),
						 lockdep_is_held(&sk->sk_callback_lock));

	WARN_ON_ONCE(flags & SK_USER_DATA_PTRMASK);

	if ((sk_user_data & flags) == flags)
		return (void *)(sk_user_data & SK_USER_DATA_PTRMASK);
	return NULL;
}


static inline void *
__rcu_dereference_sk_user_data_with_flags(const struct sock *sk,
					  uintptr_t flags)
{
	uintptr_t sk_user_data = (uintptr_t)rcu_dereference(__sk_user_data(sk));

	WARN_ON_ONCE(flags & SK_USER_DATA_PTRMASK);

	if ((sk_user_data & flags) == flags)
		return (void *)(sk_user_data & SK_USER_DATA_PTRMASK);
	return NULL;
}

#define rcu_dereference_sk_user_data(sk)				\
	__rcu_dereference_sk_user_data_with_flags(sk, 0)
#define __rcu_assign_sk_user_data_with_flags(sk, ptr, flags)		\
({									\
	uintptr_t __tmp1 = (uintptr_t)(ptr),				\
		  __tmp2 = (uintptr_t)(flags);				\
	WARN_ON_ONCE(__tmp1 & ~SK_USER_DATA_PTRMASK);			\
	WARN_ON_ONCE(__tmp2 & SK_USER_DATA_PTRMASK);			\
	rcu_assign_pointer(__sk_user_data((sk)),			\
			   __tmp1 | __tmp2);				\
})
#define rcu_assign_sk_user_data(sk, ptr)				\
	__rcu_assign_sk_user_data_with_flags(sk, ptr, 0)

static inline
struct net *sock_net(const struct sock *sk)
{
	return read_pnet(&sk->sk_net);
}

static inline
void sock_net_set(struct sock *sk, struct net *net)
{
	write_pnet(&sk->sk_net, net);
}



#define SK_NO_REUSE	0
#define SK_CAN_REUSE	1
#define SK_FORCE_REUSE	2

int sk_set_peek_off(struct sock *sk, int val);

static inline int sk_peek_offset(const struct sock *sk, int flags)
{
	if (unlikely(flags & MSG_PEEK)) {
		return READ_ONCE(sk->sk_peek_off);
	}

	return 0;
}

static inline void sk_peek_offset_bwd(struct sock *sk, int val)
{
	s32 off = READ_ONCE(sk->sk_peek_off);

	if (unlikely(off >= 0)) {
		off = max_t(s32, off - val, 0);
		WRITE_ONCE(sk->sk_peek_off, off);
	}
}

static inline void sk_peek_offset_fwd(struct sock *sk, int val)
{
	sk_peek_offset_bwd(sk, -val);
}


static inline struct sock *sk_entry(const struct hlist_node *node)
{
	return hlist_entry(node, struct sock, sk_node);
}

static inline struct sock *__sk_head(const struct hlist_head *head)
{
	return hlist_entry(head->first, struct sock, sk_node);
}

static inline struct sock *sk_head(const struct hlist_head *head)
{
	return hlist_empty(head) ? NULL : __sk_head(head);
}

static inline struct sock *__sk_nulls_head(const struct hlist_nulls_head *head)
{
	return hlist_nulls_entry(head->first, struct sock, sk_nulls_node);
}

static inline struct sock *sk_nulls_head(const struct hlist_nulls_head *head)
{
	return hlist_nulls_empty(head) ? NULL : __sk_nulls_head(head);
}

static inline struct sock *sk_next(const struct sock *sk)
{
	return hlist_entry_safe(sk->sk_node.next, struct sock, sk_node);
}

static inline struct sock *sk_nulls_next(const struct sock *sk)
{
	return (!is_a_nulls(sk->sk_nulls_node.next)) ?
		hlist_nulls_entry(sk->sk_nulls_node.next,
				  struct sock, sk_nulls_node) :
		NULL;
}

static inline bool sk_unhashed(const struct sock *sk)
{
	return hlist_unhashed(&sk->sk_node);
}

static inline bool sk_hashed(const struct sock *sk)
{
	return !sk_unhashed(sk);
}

static inline void sk_node_init(struct hlist_node *node)
{
	node->pprev = NULL;
}

static inline void sk_nulls_node_init(struct hlist_nulls_node *node)
{
	node->pprev = NULL;
}

static inline void __sk_del_node(struct sock *sk)
{
	__hlist_del(&sk->sk_node);
}


static inline bool __sk_del_node_init(struct sock *sk)
{
	if (sk_hashed(sk)) {
		__sk_del_node(sk);
		sk_node_init(&sk->sk_node);
		return true;
	}
	return false;
}



static __always_inline void sock_hold(struct sock *sk)
{
	refcount_inc(&sk->sk_refcnt);
}


static __always_inline void __sock_put(struct sock *sk)
{
	refcount_dec(&sk->sk_refcnt);
}

static inline bool sk_del_node_init(struct sock *sk)
{
	bool rc = __sk_del_node_init(sk);

	if (rc) {
		
		WARN_ON(refcount_read(&sk->sk_refcnt) == 1);
		__sock_put(sk);
	}
	return rc;
}
#define sk_del_node_init_rcu(sk)	sk_del_node_init(sk)

static inline bool __sk_nulls_del_node_init_rcu(struct sock *sk)
{
	if (sk_hashed(sk)) {
		hlist_nulls_del_init_rcu(&sk->sk_nulls_node);
		return true;
	}
	return false;
}

static inline bool sk_nulls_del_node_init_rcu(struct sock *sk)
{
	bool rc = __sk_nulls_del_node_init_rcu(sk);

	if (rc) {
		
		WARN_ON(refcount_read(&sk->sk_refcnt) == 1);
		__sock_put(sk);
	}
	return rc;
}

static inline void __sk_add_node(struct sock *sk, struct hlist_head *list)
{
	hlist_add_head(&sk->sk_node, list);
}

static inline void sk_add_node(struct sock *sk, struct hlist_head *list)
{
	sock_hold(sk);
	__sk_add_node(sk, list);
}

static inline void sk_add_node_rcu(struct sock *sk, struct hlist_head *list)
{
	sock_hold(sk);
	if (IS_ENABLED(CONFIG_IPV6) && sk->sk_reuseport &&
	    sk->sk_family == AF_INET6)
		hlist_add_tail_rcu(&sk->sk_node, list);
	else
		hlist_add_head_rcu(&sk->sk_node, list);
}

static inline void sk_add_node_tail_rcu(struct sock *sk, struct hlist_head *list)
{
	sock_hold(sk);
	hlist_add_tail_rcu(&sk->sk_node, list);
}

static inline void __sk_nulls_add_node_rcu(struct sock *sk, struct hlist_nulls_head *list)
{
	hlist_nulls_add_head_rcu(&sk->sk_nulls_node, list);
}

static inline void __sk_nulls_add_node_tail_rcu(struct sock *sk, struct hlist_nulls_head *list)
{
	hlist_nulls_add_tail_rcu(&sk->sk_nulls_node, list);
}

static inline void sk_nulls_add_node_rcu(struct sock *sk, struct hlist_nulls_head *list)
{
	sock_hold(sk);
	__sk_nulls_add_node_rcu(sk, list);
}

static inline void __sk_del_bind_node(struct sock *sk)
{
	__hlist_del(&sk->sk_bind_node);
}

static inline void sk_add_bind_node(struct sock *sk,
					struct hlist_head *list)
{
	hlist_add_head(&sk->sk_bind_node, list);
}

#define sk_for_each(__sk, list) \
	hlist_for_each_entry(__sk, list, sk_node)
#define sk_for_each_rcu(__sk, list) \
	hlist_for_each_entry_rcu(__sk, list, sk_node)
#define sk_nulls_for_each(__sk, node, list) \
	hlist_nulls_for_each_entry(__sk, node, list, sk_nulls_node)
#define sk_nulls_for_each_rcu(__sk, node, list) \
	hlist_nulls_for_each_entry_rcu(__sk, node, list, sk_nulls_node)
#define sk_for_each_from(__sk) \
	hlist_for_each_entry_from(__sk, sk_node)
#define sk_nulls_for_each_from(__sk, node) \
	if (__sk && ({ node = &(__sk)->sk_nulls_node; 1; })) \
		hlist_nulls_for_each_entry_from(__sk, node, sk_nulls_node)
#define sk_for_each_safe(__sk, tmp, list) \
	hlist_for_each_entry_safe(__sk, tmp, list, sk_node)
#define sk_for_each_bound(__sk, list) \
	hlist_for_each_entry(__sk, list, sk_bind_node)


#define sk_for_each_entry_offset_rcu(tpos, pos, head, offset)		       \
	for (pos = rcu_dereference(hlist_first_rcu(head));		       \
	     pos != NULL &&						       \
		({ tpos = (typeof(*tpos) *)((void *)pos - offset); 1;});       \
	     pos = rcu_dereference(hlist_next_rcu(pos)))

static inline struct user_namespace *sk_user_ns(const struct sock *sk)
{
	
	return sk->sk_socket->file->f_cred->user_ns;
}


enum sock_flags {
	SOCK_DEAD,
	SOCK_DONE,
	SOCK_URGINLINE,
	SOCK_KEEPOPEN,
	SOCK_LINGER,
	SOCK_DESTROY,
	SOCK_BROADCAST,
	SOCK_TIMESTAMP,
	SOCK_ZAPPED,
	SOCK_USE_WRITE_QUEUE, 
	SOCK_DBG, 
	SOCK_RCVTSTAMP, 
	SOCK_RCVTSTAMPNS, 
	SOCK_LOCALROUTE, 
	SOCK_MEMALLOC, 
	SOCK_TIMESTAMPING_RX_SOFTWARE,  
	SOCK_FASYNC, 
	SOCK_RXQ_OVFL,
	SOCK_ZEROCOPY, 
	SOCK_WIFI_STATUS, 
	SOCK_NOFCS, 
	SOCK_FILTER_LOCKED, 
	SOCK_SELECT_ERR_QUEUE, 
	SOCK_RCU_FREE, 
	SOCK_TXTIME,
	SOCK_XDP, 
	SOCK_TSTAMP_NEW, 
	SOCK_RCVMARK, 
};

#define SK_FLAGS_TIMESTAMP ((1UL << SOCK_TIMESTAMP) | (1UL << SOCK_TIMESTAMPING_RX_SOFTWARE))

static inline void sock_copy_flags(struct sock *nsk, const struct sock *osk)
{
	nsk->sk_flags = osk->sk_flags;
}

static inline void sock_set_flag(struct sock *sk, enum sock_flags flag)
{
	__set_bit(flag, &sk->sk_flags);
}

static inline void sock_reset_flag(struct sock *sk, enum sock_flags flag)
{
	__clear_bit(flag, &sk->sk_flags);
}

static inline void sock_valbool_flag(struct sock *sk, enum sock_flags bit,
				     int valbool)
{
	if (valbool)
		sock_set_flag(sk, bit);
	else
		sock_reset_flag(sk, bit);
}

static inline bool sock_flag(const struct sock *sk, enum sock_flags flag)
{
	return test_bit(flag, &sk->sk_flags);
}

#ifdef CONFIG_NET
DECLARE_STATIC_KEY_FALSE(memalloc_socks_key);
static inline int sk_memalloc_socks(void)
{
	return static_branch_unlikely(&memalloc_socks_key);
}

void __receive_sock(struct file *file);
#else

static inline int sk_memalloc_socks(void)
{
	return 0;
}

static inline void __receive_sock(struct file *file)
{ }
#endif

static inline gfp_t sk_gfp_mask(const struct sock *sk, gfp_t gfp_mask)
{
	return gfp_mask | (sk->sk_allocation & __GFP_MEMALLOC);
}

static inline void sk_acceptq_removed(struct sock *sk)
{
	WRITE_ONCE(sk->sk_ack_backlog, sk->sk_ack_backlog - 1);
}

static inline void sk_acceptq_added(struct sock *sk)
{
	WRITE_ONCE(sk->sk_ack_backlog, sk->sk_ack_backlog + 1);
}


static inline bool sk_acceptq_is_full(const struct sock *sk)
{
	return READ_ONCE(sk->sk_ack_backlog) > READ_ONCE(sk->sk_max_ack_backlog);
}


static inline int sk_stream_min_wspace(const struct sock *sk)
{
	return READ_ONCE(sk->sk_wmem_queued) >> 1;
}

static inline int sk_stream_wspace(const struct sock *sk)
{
	return READ_ONCE(sk->sk_sndbuf) - READ_ONCE(sk->sk_wmem_queued);
}

static inline void sk_wmem_queued_add(struct sock *sk, int val)
{
	WRITE_ONCE(sk->sk_wmem_queued, sk->sk_wmem_queued + val);
}

void sk_stream_write_space(struct sock *sk);


static inline void __sk_add_backlog(struct sock *sk, struct sk_buff *skb)
{
	
	skb_dst_force(skb);

	if (!sk->sk_backlog.tail)
		WRITE_ONCE(sk->sk_backlog.head, skb);
	else
		sk->sk_backlog.tail->next = skb;

	WRITE_ONCE(sk->sk_backlog.tail, skb);
	skb->next = NULL;
}


static inline bool sk_rcvqueues_full(const struct sock *sk, unsigned int limit)
{
	unsigned int qsize = sk->sk_backlog.len + atomic_read(&sk->sk_rmem_alloc);

	return qsize > limit;
}


static inline __must_check int sk_add_backlog(struct sock *sk, struct sk_buff *skb,
					      unsigned int limit)
{
	if (sk_rcvqueues_full(sk, limit))
		return -ENOBUFS;

	
	if (skb_pfmemalloc(skb) && !sock_flag(sk, SOCK_MEMALLOC))
		return -ENOMEM;

	__sk_add_backlog(sk, skb);
	sk->sk_backlog.len += skb->truesize;
	return 0;
}

int __sk_backlog_rcv(struct sock *sk, struct sk_buff *skb);

INDIRECT_CALLABLE_DECLARE(int tcp_v4_do_rcv(struct sock *sk, struct sk_buff *skb));
INDIRECT_CALLABLE_DECLARE(int tcp_v6_do_rcv(struct sock *sk, struct sk_buff *skb));

static inline int sk_backlog_rcv(struct sock *sk, struct sk_buff *skb)
{
	if (sk_memalloc_socks() && skb_pfmemalloc(skb))
		return __sk_backlog_rcv(sk, skb);

	return INDIRECT_CALL_INET(sk->sk_backlog_rcv,
				  tcp_v6_do_rcv,
				  tcp_v4_do_rcv,
				  sk, skb);
}

static inline void sk_incoming_cpu_update(struct sock *sk)
{
	int cpu = raw_smp_processor_id();

	if (unlikely(READ_ONCE(sk->sk_incoming_cpu) != cpu))
		WRITE_ONCE(sk->sk_incoming_cpu, cpu);
}

static inline void sock_rps_record_flow_hash(__u32 hash)
{
#ifdef CONFIG_RPS
	struct rps_sock_flow_table *sock_flow_table;

	rcu_read_lock();
	sock_flow_table = rcu_dereference(rps_sock_flow_table);
	rps_record_sock_flow(sock_flow_table, hash);
	rcu_read_unlock();
#endif
}

static inline void sock_rps_record_flow(const struct sock *sk)
{
#ifdef CONFIG_RPS
	if (static_branch_unlikely(&rfs_needed)) {
		
		if (sk->sk_state == TCP_ESTABLISHED)
			sock_rps_record_flow_hash(sk->sk_rxhash);
	}
#endif
}

static inline void sock_rps_save_rxhash(struct sock *sk,
					const struct sk_buff *skb)
{
#ifdef CONFIG_RPS
	if (unlikely(sk->sk_rxhash != skb->hash))
		sk->sk_rxhash = skb->hash;
#endif
}

static inline void sock_rps_reset_rxhash(struct sock *sk)
{
#ifdef CONFIG_RPS
	sk->sk_rxhash = 0;
#endif
}

#define sk_wait_event(__sk, __timeo, __condition, __wait)		\
	({	int __rc;						\
		release_sock(__sk);					\
		__rc = __condition;					\
		if (!__rc) {						\
			*(__timeo) = wait_woken(__wait,			\
						TASK_INTERRUPTIBLE,	\
						*(__timeo));		\
		}							\
		sched_annotate_sleep();					\
		lock_sock(__sk);					\
		__rc = __condition;					\
		__rc;							\
	})

int sk_stream_wait_connect(struct sock *sk, long *timeo_p);
int sk_stream_wait_memory(struct sock *sk, long *timeo_p);
void sk_stream_wait_close(struct sock *sk, long timeo_p);
int sk_stream_error(struct sock *sk, int flags, int err);
void sk_stream_kill_queues(struct sock *sk);
void sk_set_memalloc(struct sock *sk);
void sk_clear_memalloc(struct sock *sk);

void __sk_flush_backlog(struct sock *sk);

static inline bool sk_flush_backlog(struct sock *sk)
{
	if (unlikely(READ_ONCE(sk->sk_backlog.tail))) {
		__sk_flush_backlog(sk);
		return true;
	}
	return false;
}

int sk_wait_data(struct sock *sk, long *timeo, const struct sk_buff *skb);

struct request_sock_ops;
struct timewait_sock_ops;
struct inet_hashinfo;
struct raw_hashinfo;
struct smc_hashinfo;
struct module;
struct sk_psock;


static inline void sk_prot_clear_nulls(struct sock *sk, int size)
{
	if (offsetof(struct sock, sk_node.next) != 0)
		memset(sk, 0, offsetof(struct sock, sk_node.next));
	memset(&sk->sk_node.pprev, 0,
	       size - offsetof(struct sock, sk_node.pprev));
}


struct proto {
	void			(*close)(struct sock *sk,
					long timeout);
	int			(*pre_connect)(struct sock *sk,
					struct sockaddr *uaddr,
					int addr_len);
	int			(*connect)(struct sock *sk,
					struct sockaddr *uaddr,
					int addr_len);
	int			(*disconnect)(struct sock *sk, int flags);

	struct sock *		(*accept)(struct sock *sk, int flags, int *err,
					  bool kern);

	int			(*ioctl)(struct sock *sk, int cmd,
					 unsigned long arg);
	int			(*init)(struct sock *sk);
	void			(*destroy)(struct sock *sk);
	void			(*shutdown)(struct sock *sk, int how);
	int			(*setsockopt)(struct sock *sk, int level,
					int optname, sockptr_t optval,
					unsigned int optlen);
	int			(*getsockopt)(struct sock *sk, int level,
					int optname, char __user *optval,
					int __user *option);
	void			(*keepalive)(struct sock *sk, int valbool);
#ifdef CONFIG_COMPAT
	int			(*compat_ioctl)(struct sock *sk,
					unsigned int cmd, unsigned long arg);
#endif
	int			(*sendmsg)(struct sock *sk, struct msghdr *msg,
					   size_t len);
	int			(*recvmsg)(struct sock *sk, struct msghdr *msg,
					   size_t len, int flags, int *addr_len);
	int			(*sendpage)(struct sock *sk, struct page *page,
					int offset, size_t size, int flags);
	int			(*bind)(struct sock *sk,
					struct sockaddr *addr, int addr_len);
	int			(*bind_add)(struct sock *sk,
					struct sockaddr *addr, int addr_len);

	int			(*backlog_rcv) (struct sock *sk,
						struct sk_buff *skb);
	bool			(*bpf_bypass_getsockopt)(int level,
							 int optname);

	void		(*release_cb)(struct sock *sk);

	
	int			(*hash)(struct sock *sk);
	void			(*unhash)(struct sock *sk);
	void			(*rehash)(struct sock *sk);
	int			(*get_port)(struct sock *sk, unsigned short snum);
	void			(*put_port)(struct sock *sk);
#ifdef CONFIG_BPF_SYSCALL
	int			(*psock_update_sk_prot)(struct sock *sk,
							struct sk_psock *psock,
							bool restore);
#endif

	
#ifdef CONFIG_PROC_FS
	unsigned int		inuse_idx;
#endif

#if IS_ENABLED(CONFIG_MPTCP)
	int			(*forward_alloc_get)(const struct sock *sk);
#endif

	bool			(*stream_memory_free)(const struct sock *sk, int wake);
	bool			(*sock_is_readable)(struct sock *sk);
	
	void			(*enter_memory_pressure)(struct sock *sk);
	void			(*leave_memory_pressure)(struct sock *sk);
	atomic_long_t		*memory_allocated;	
	int  __percpu		*per_cpu_fw_alloc;
	struct percpu_counter	*sockets_allocated;	

	
	unsigned long		*memory_pressure;
	long			*sysctl_mem;

	int			*sysctl_wmem;
	int			*sysctl_rmem;
	u32			sysctl_wmem_offset;
	u32			sysctl_rmem_offset;

	int			max_header;
	bool			no_autobind;

	struct kmem_cache	*slab;
	unsigned int		obj_size;
	slab_flags_t		slab_flags;
	unsigned int		useroffset;	
	unsigned int		usersize;	

	unsigned int __percpu	*orphan_count;

	struct request_sock_ops	*rsk_prot;
	struct timewait_sock_ops *twsk_prot;

	union {
		struct inet_hashinfo	*hashinfo;
		struct udp_table	*udp_table;
		struct raw_hashinfo	*raw_hash;
		struct smc_hashinfo	*smc_hash;
	} h;

	struct module		*owner;

	char			name[32];

	struct list_head	node;
#ifdef SOCK_REFCNT_DEBUG
	atomic_t		socks;
#endif
	int			(*diag_destroy)(struct sock *sk, int err);
} __randomize_layout;

int proto_register(struct proto *prot, int alloc_slab);
void proto_unregister(struct proto *prot);
int sock_load_diag_module(int family, int protocol);

#ifdef SOCK_REFCNT_DEBUG
static inline void sk_refcnt_debug_inc(struct sock *sk)
{
	atomic_inc(&sk->sk_prot->socks);
}

static inline void sk_refcnt_debug_dec(struct sock *sk)
{
	atomic_dec(&sk->sk_prot->socks);
	printk(KERN_DEBUG "%s socket %p released, %d are still alive\n",
	       sk->sk_prot->name, sk, atomic_read(&sk->sk_prot->socks));
}

static inline void sk_refcnt_debug_release(const struct sock *sk)
{
	if (refcount_read(&sk->sk_refcnt) != 1)
		printk(KERN_DEBUG "Destruction of the %s socket %p delayed, refcnt=%d\n",
		       sk->sk_prot->name, sk, refcount_read(&sk->sk_refcnt));
}
#else 
#define sk_refcnt_debug_inc(sk) do { } while (0)
#define sk_refcnt_debug_dec(sk) do { } while (0)
#define sk_refcnt_debug_release(sk) do { } while (0)
#endif 

INDIRECT_CALLABLE_DECLARE(bool tcp_stream_memory_free(const struct sock *sk, int wake));

static inline int sk_forward_alloc_get(const struct sock *sk)
{
#if IS_ENABLED(CONFIG_MPTCP)
	if (sk->sk_prot->forward_alloc_get)
		return sk->sk_prot->forward_alloc_get(sk);
#endif
	return sk->sk_forward_alloc;
}

static inline bool __sk_stream_memory_free(const struct sock *sk, int wake)
{
	if (READ_ONCE(sk->sk_wmem_queued) >= READ_ONCE(sk->sk_sndbuf))
		return false;

	return sk->sk_prot->stream_memory_free ?
		INDIRECT_CALL_INET_1(sk->sk_prot->stream_memory_free,
				     tcp_stream_memory_free, sk, wake) : true;
}

static inline bool sk_stream_memory_free(const struct sock *sk)
{
	return __sk_stream_memory_free(sk, 0);
}

static inline bool __sk_stream_is_writeable(const struct sock *sk, int wake)
{
	return sk_stream_wspace(sk) >= sk_stream_min_wspace(sk) &&
	       __sk_stream_memory_free(sk, wake);
}

static inline bool sk_stream_is_writeable(const struct sock *sk)
{
	return __sk_stream_is_writeable(sk, 0);
}

static inline int sk_under_cgroup_hierarchy(struct sock *sk,
					    struct cgroup *ancestor)
{
#ifdef CONFIG_SOCK_CGROUP_DATA
	return cgroup_is_descendant(sock_cgroup_ptr(&sk->sk_cgrp_data),
				    ancestor);
#else
	return -ENOTSUPP;
#endif
}

static inline bool sk_has_memory_pressure(const struct sock *sk)
{
	return sk->sk_prot->memory_pressure != NULL;
}

static inline bool sk_under_memory_pressure(const struct sock *sk)
{
	if (!sk->sk_prot->memory_pressure)
		return false;

	if (mem_cgroup_sockets_enabled && sk->sk_memcg &&
	    mem_cgroup_under_socket_pressure(sk->sk_memcg))
		return true;

	return !!*sk->sk_prot->memory_pressure;
}

static inline long
proto_memory_allocated(const struct proto *prot)
{
	return max(0L, atomic_long_read(prot->memory_allocated));
}

static inline long
sk_memory_allocated(const struct sock *sk)
{
	return proto_memory_allocated(sk->sk_prot);
}


#define SK_MEMORY_PCPU_RESERVE (1 << (20 - PAGE_SHIFT))

static inline void
sk_memory_allocated_add(struct sock *sk, int amt)
{
	int local_reserve;

	preempt_disable();
	local_reserve = __this_cpu_add_return(*sk->sk_prot->per_cpu_fw_alloc, amt);
	if (local_reserve >= SK_MEMORY_PCPU_RESERVE) {
		__this_cpu_sub(*sk->sk_prot->per_cpu_fw_alloc, local_reserve);
		atomic_long_add(local_reserve, sk->sk_prot->memory_allocated);
	}
	preempt_enable();
}

static inline void
sk_memory_allocated_sub(struct sock *sk, int amt)
{
	int local_reserve;

	preempt_disable();
	local_reserve = __this_cpu_sub_return(*sk->sk_prot->per_cpu_fw_alloc, amt);
	if (local_reserve <= -SK_MEMORY_PCPU_RESERVE) {
		__this_cpu_sub(*sk->sk_prot->per_cpu_fw_alloc, local_reserve);
		atomic_long_add(local_reserve, sk->sk_prot->memory_allocated);
	}
	preempt_enable();
}

#define SK_ALLOC_PERCPU_COUNTER_BATCH 16

static inline void sk_sockets_allocated_dec(struct sock *sk)
{
	percpu_counter_add_batch(sk->sk_prot->sockets_allocated, -1,
				 SK_ALLOC_PERCPU_COUNTER_BATCH);
}

static inline void sk_sockets_allocated_inc(struct sock *sk)
{
	percpu_counter_add_batch(sk->sk_prot->sockets_allocated, 1,
				 SK_ALLOC_PERCPU_COUNTER_BATCH);
}

static inline u64
sk_sockets_allocated_read_positive(struct sock *sk)
{
	return percpu_counter_read_positive(sk->sk_prot->sockets_allocated);
}

static inline int
proto_sockets_allocated_sum_positive(struct proto *prot)
{
	return percpu_counter_sum_positive(prot->sockets_allocated);
}

static inline bool
proto_memory_pressure(struct proto *prot)
{
	if (!prot->memory_pressure)
		return false;
	return !!*prot->memory_pressure;
}


#ifdef CONFIG_PROC_FS
#define PROTO_INUSE_NR	64	
struct prot_inuse {
	int all;
	int val[PROTO_INUSE_NR];
};

static inline void sock_prot_inuse_add(const struct net *net,
				       const struct proto *prot, int val)
{
	this_cpu_add(net->core.prot_inuse->val[prot->inuse_idx], val);
}

static inline void sock_inuse_add(const struct net *net, int val)
{
	this_cpu_add(net->core.prot_inuse->all, val);
}

int sock_prot_inuse_get(struct net *net, struct proto *proto);
int sock_inuse_get(struct net *net);
#else
static inline void sock_prot_inuse_add(const struct net *net,
				       const struct proto *prot, int val)
{
}

static inline void sock_inuse_add(const struct net *net, int val)
{
}
#endif



static inline int __sk_prot_rehash(struct sock *sk)
{
	sk->sk_prot->unhash(sk);
	return sk->sk_prot->hash(sk);
}


#define SOCK_DESTROY_TIME (10*HZ)


#define PROT_SOCK	1024

#define SHUTDOWN_MASK	3
#define RCV_SHUTDOWN	1
#define SEND_SHUTDOWN	2

#define SOCK_BINDADDR_LOCK	4
#define SOCK_BINDPORT_LOCK	8

struct socket_alloc {
	struct socket socket;
	struct inode vfs_inode;
};

static inline struct socket *SOCKET_I(struct inode *inode)
{
	return &container_of(inode, struct socket_alloc, vfs_inode)->socket;
}

static inline struct inode *SOCK_INODE(struct socket *socket)
{
	return &container_of(socket, struct socket_alloc, socket)->vfs_inode;
}


int __sk_mem_raise_allocated(struct sock *sk, int size, int amt, int kind);
int __sk_mem_schedule(struct sock *sk, int size, int kind);
void __sk_mem_reduce_allocated(struct sock *sk, int amount);
void __sk_mem_reclaim(struct sock *sk, int amount);

#define SK_MEM_SEND	0
#define SK_MEM_RECV	1


static inline long sk_prot_mem_limits(const struct sock *sk, int index)
{
	return READ_ONCE(sk->sk_prot->sysctl_mem[index]);
}

static inline int sk_mem_pages(int amt)
{
	return (amt + PAGE_SIZE - 1) >> PAGE_SHIFT;
}

static inline bool sk_has_account(struct sock *sk)
{
	
	return !!sk->sk_prot->memory_allocated;
}

static inline bool sk_wmem_schedule(struct sock *sk, int size)
{
	int delta;

	if (!sk_has_account(sk))
		return true;
	delta = size - sk->sk_forward_alloc;
	return delta <= 0 || __sk_mem_schedule(sk, delta, SK_MEM_SEND);
}

static inline bool
sk_rmem_schedule(struct sock *sk, struct sk_buff *skb, int size)
{
	int delta;

	if (!sk_has_account(sk))
		return true;
	delta = size - sk->sk_forward_alloc;
	return delta <= 0 || __sk_mem_schedule(sk, delta, SK_MEM_RECV) ||
		skb_pfmemalloc(skb);
}

static inline int sk_unused_reserved_mem(const struct sock *sk)
{
	int unused_mem;

	if (likely(!sk->sk_reserved_mem))
		return 0;

	unused_mem = sk->sk_reserved_mem - sk->sk_wmem_queued -
			atomic_read(&sk->sk_rmem_alloc);

	return unused_mem > 0 ? unused_mem : 0;
}

static inline void sk_mem_reclaim(struct sock *sk)
{
	int reclaimable;

	if (!sk_has_account(sk))
		return;

	reclaimable = sk->sk_forward_alloc - sk_unused_reserved_mem(sk);

	if (reclaimable >= (int)PAGE_SIZE)
		__sk_mem_reclaim(sk, reclaimable);
}

static inline void sk_mem_reclaim_final(struct sock *sk)
{
	sk->sk_reserved_mem = 0;
	sk_mem_reclaim(sk);
}

static inline void sk_mem_charge(struct sock *sk, int size)
{
	if (!sk_has_account(sk))
		return;
	sk->sk_forward_alloc -= size;
}

static inline void sk_mem_uncharge(struct sock *sk, int size)
{
	if (!sk_has_account(sk))
		return;
	sk->sk_forward_alloc += size;
	sk_mem_reclaim(sk);
}


#define sock_lock_init_class_and_name(sk, sname, skey, name, key)	\
do {									\
	sk->sk_lock.owned = 0;						\
	init_waitqueue_head(&sk->sk_lock.wq);				\
	spin_lock_init(&(sk)->sk_lock.slock);				\
	debug_check_no_locks_freed((void *)&(sk)->sk_lock,		\
			sizeof((sk)->sk_lock));				\
	lockdep_set_class_and_name(&(sk)->sk_lock.slock,		\
				(skey), (sname));				\
	lockdep_init_map(&(sk)->sk_lock.dep_map, (name), (key), 0);	\
} while (0)

static inline bool lockdep_sock_is_held(const struct sock *sk)
{
	return lockdep_is_held(&sk->sk_lock) ||
	       lockdep_is_held(&sk->sk_lock.slock);
}

void lock_sock_nested(struct sock *sk, int subclass);

static inline void lock_sock(struct sock *sk)
{
	lock_sock_nested(sk, 0);
}

void __lock_sock(struct sock *sk);
void __release_sock(struct sock *sk);
void release_sock(struct sock *sk);


#define bh_lock_sock(__sk)	spin_lock(&((__sk)->sk_lock.slock))
#define bh_lock_sock_nested(__sk) \
				spin_lock_nested(&((__sk)->sk_lock.slock), \
				SINGLE_DEPTH_NESTING)
#define bh_unlock_sock(__sk)	spin_unlock(&((__sk)->sk_lock.slock))

bool __lock_sock_fast(struct sock *sk) __acquires(&sk->sk_lock.slock);


static inline bool lock_sock_fast(struct sock *sk)
{
	
	mutex_acquire(&sk->sk_lock.dep_map, 0, 0, _RET_IP_);

	return __lock_sock_fast(sk);
}


static inline bool lock_sock_fast_nested(struct sock *sk)
{
	mutex_acquire(&sk->sk_lock.dep_map, SINGLE_DEPTH_NESTING, 0, _RET_IP_);

	return __lock_sock_fast(sk);
}


static inline void unlock_sock_fast(struct sock *sk, bool slow)
	__releases(&sk->sk_lock.slock)
{
	if (slow) {
		release_sock(sk);
		__release(&sk->sk_lock.slock);
	} else {
		mutex_release(&sk->sk_lock.dep_map, _RET_IP_);
		spin_unlock_bh(&sk->sk_lock.slock);
	}
}



static inline void sock_owned_by_me(const struct sock *sk)
{
#ifdef CONFIG_LOCKDEP
	WARN_ON_ONCE(!lockdep_sock_is_held(sk) && debug_locks);
#endif
}

static inline bool sock_owned_by_user(const struct sock *sk)
{
	sock_owned_by_me(sk);
	return sk->sk_lock.owned;
}

static inline bool sock_owned_by_user_nocheck(const struct sock *sk)
{
	return sk->sk_lock.owned;
}

static inline void sock_release_ownership(struct sock *sk)
{
	if (sock_owned_by_user_nocheck(sk)) {
		sk->sk_lock.owned = 0;

		
		mutex_release(&sk->sk_lock.dep_map, _RET_IP_);
	}
}


static inline bool sock_allow_reclassification(const struct sock *csk)
{
	struct sock *sk = (struct sock *)csk;

	return !sock_owned_by_user_nocheck(sk) &&
		!spin_is_locked(&sk->sk_lock.slock);
}

struct sock *sk_alloc(struct net *net, int family, gfp_t priority,
		      struct proto *prot, int kern);
void sk_free(struct sock *sk);
void sk_destruct(struct sock *sk);
struct sock *sk_clone_lock(const struct sock *sk, const gfp_t priority);
void sk_free_unlock_clone(struct sock *sk);

struct sk_buff *sock_wmalloc(struct sock *sk, unsigned long size, int force,
			     gfp_t priority);
void __sock_wfree(struct sk_buff *skb);
void sock_wfree(struct sk_buff *skb);
struct sk_buff *sock_omalloc(struct sock *sk, unsigned long size,
			     gfp_t priority);
void skb_orphan_partial(struct sk_buff *skb);
void sock_rfree(struct sk_buff *skb);
void sock_efree(struct sk_buff *skb);
#ifdef CONFIG_INET
void sock_edemux(struct sk_buff *skb);
void sock_pfree(struct sk_buff *skb);
#else
#define sock_edemux sock_efree
#endif

int sock_setsockopt(struct socket *sock, int level, int op,
		    sockptr_t optval, unsigned int optlen);

int sock_getsockopt(struct socket *sock, int level, int op,
		    char __user *optval, int __user *optlen);
int sock_gettstamp(struct socket *sock, void __user *userstamp,
		   bool timeval, bool time32);
struct sk_buff *sock_alloc_send_pskb(struct sock *sk, unsigned long header_len,
				     unsigned long data_len, int noblock,
				     int *errcode, int max_page_order);

static inline struct sk_buff *sock_alloc_send_skb(struct sock *sk,
						  unsigned long size,
						  int noblock, int *errcode)
{
	return sock_alloc_send_pskb(sk, size, 0, noblock, errcode, 0);
}

void *sock_kmalloc(struct sock *sk, int size, gfp_t priority);
void sock_kfree_s(struct sock *sk, void *mem, int size);
void sock_kzfree_s(struct sock *sk, void *mem, int size);
void sk_send_sigurg(struct sock *sk);

static inline void sock_replace_proto(struct sock *sk, struct proto *proto)
{
	if (sk->sk_socket)
		clear_bit(SOCK_SUPPORT_ZC, &sk->sk_socket->flags);
	WRITE_ONCE(sk->sk_prot, proto);
}

struct sockcm_cookie {
	u64 transmit_time;
	u32 mark;
	u16 tsflags;
};

static inline void sockcm_init(struct sockcm_cookie *sockc,
			       const struct sock *sk)
{
	*sockc = (struct sockcm_cookie) { .tsflags = sk->sk_tsflags };
}

int __sock_cmsg_send(struct sock *sk, struct msghdr *msg, struct cmsghdr *cmsg,
		     struct sockcm_cookie *sockc);
int sock_cmsg_send(struct sock *sk, struct msghdr *msg,
		   struct sockcm_cookie *sockc);


int sock_no_bind(struct socket *, struct sockaddr *, int);
int sock_no_connect(struct socket *, struct sockaddr *, int, int);
int sock_no_socketpair(struct socket *, struct socket *);
int sock_no_accept(struct socket *, struct socket *, int, bool);
int sock_no_getname(struct socket *, struct sockaddr *, int);
int sock_no_ioctl(struct socket *, unsigned int, unsigned long);
int sock_no_listen(struct socket *, int);
int sock_no_shutdown(struct socket *, int);
int sock_no_sendmsg(struct socket *, struct msghdr *, size_t);
int sock_no_sendmsg_locked(struct sock *sk, struct msghdr *msg, size_t len);
int sock_no_recvmsg(struct socket *, struct msghdr *, size_t, int);
int sock_no_mmap(struct file *file, struct socket *sock,
		 struct vm_area_struct *vma);
ssize_t sock_no_sendpage(struct socket *sock, struct page *page, int offset,
			 size_t size, int flags);
ssize_t sock_no_sendpage_locked(struct sock *sk, struct page *page,
				int offset, size_t size, int flags);


int sock_common_getsockopt(struct socket *sock, int level, int optname,
				  char __user *optval, int __user *optlen);
int sock_common_recvmsg(struct socket *sock, struct msghdr *msg, size_t size,
			int flags);
int sock_common_setsockopt(struct socket *sock, int level, int optname,
			   sockptr_t optval, unsigned int optlen);

void sk_common_release(struct sock *sk);




void sock_init_data(struct socket *sock, struct sock *sk);




static inline void sock_put(struct sock *sk)
{
	if (refcount_dec_and_test(&sk->sk_refcnt))
		sk_free(sk);
}

void sock_gen_put(struct sock *sk);

int __sk_receive_skb(struct sock *sk, struct sk_buff *skb, const int nested,
		     unsigned int trim_cap, bool refcounted);
static inline int sk_receive_skb(struct sock *sk, struct sk_buff *skb,
				 const int nested)
{
	return __sk_receive_skb(sk, skb, nested, 1, true);
}

static inline void sk_tx_queue_set(struct sock *sk, int tx_queue)
{
	
	if (WARN_ON_ONCE((unsigned short)tx_queue >= USHRT_MAX))
		return;
	sk->sk_tx_queue_mapping = tx_queue;
}

#define NO_QUEUE_MAPPING	USHRT_MAX

static inline void sk_tx_queue_clear(struct sock *sk)
{
	sk->sk_tx_queue_mapping = NO_QUEUE_MAPPING;
}

static inline int sk_tx_queue_get(const struct sock *sk)
{
	if (sk && sk->sk_tx_queue_mapping != NO_QUEUE_MAPPING)
		return sk->sk_tx_queue_mapping;

	return -1;
}

static inline void __sk_rx_queue_set(struct sock *sk,
				     const struct sk_buff *skb,
				     bool force_set)
{
#ifdef CONFIG_SOCK_RX_QUEUE_MAPPING
	if (skb_rx_queue_recorded(skb)) {
		u16 rx_queue = skb_get_rx_queue(skb);

		if (force_set ||
		    unlikely(READ_ONCE(sk->sk_rx_queue_mapping) != rx_queue))
			WRITE_ONCE(sk->sk_rx_queue_mapping, rx_queue);
	}
#endif
}

static inline void sk_rx_queue_set(struct sock *sk, const struct sk_buff *skb)
{
	__sk_rx_queue_set(sk, skb, true);
}

static inline void sk_rx_queue_update(struct sock *sk, const struct sk_buff *skb)
{
	__sk_rx_queue_set(sk, skb, false);
}

static inline void sk_rx_queue_clear(struct sock *sk)
{
#ifdef CONFIG_SOCK_RX_QUEUE_MAPPING
	WRITE_ONCE(sk->sk_rx_queue_mapping, NO_QUEUE_MAPPING);
#endif
}

static inline int sk_rx_queue_get(const struct sock *sk)
{
#ifdef CONFIG_SOCK_RX_QUEUE_MAPPING
	if (sk) {
		int res = READ_ONCE(sk->sk_rx_queue_mapping);

		if (res != NO_QUEUE_MAPPING)
			return res;
	}
#endif

	return -1;
}

static inline void sk_set_socket(struct sock *sk, struct socket *sock)
{
	sk->sk_socket = sock;
}

static inline wait_queue_head_t *sk_sleep(struct sock *sk)
{
	BUILD_BUG_ON(offsetof(struct socket_wq, wait) != 0);
	return &rcu_dereference_raw(sk->sk_wq)->wait;
}

static inline void sock_orphan(struct sock *sk)
{
	write_lock_bh(&sk->sk_callback_lock);
	sock_set_flag(sk, SOCK_DEAD);
	sk_set_socket(sk, NULL);
	sk->sk_wq  = NULL;
	write_unlock_bh(&sk->sk_callback_lock);
}

static inline void sock_graft(struct sock *sk, struct socket *parent)
{
	WARN_ON(parent->sk);
	write_lock_bh(&sk->sk_callback_lock);
	rcu_assign_pointer(sk->sk_wq, &parent->wq);
	parent->sk = sk;
	sk_set_socket(sk, parent);
	sk->sk_uid = SOCK_INODE(parent)->i_uid;
	security_sock_graft(sk, parent);
	write_unlock_bh(&sk->sk_callback_lock);
}

kuid_t sock_i_uid(struct sock *sk);
unsigned long sock_i_ino(struct sock *sk);

static inline kuid_t sock_net_uid(const struct net *net, const struct sock *sk)
{
	return sk ? sk->sk_uid : make_kuid(net->user_ns, 0);
}

static inline u32 net_tx_rndhash(void)
{
	u32 v = prandom_u32();

	return v ?: 1;
}

static inline void sk_set_txhash(struct sock *sk)
{
	
	WRITE_ONCE(sk->sk_txhash, net_tx_rndhash());
}

static inline bool sk_rethink_txhash(struct sock *sk)
{
	if (sk->sk_txhash && sk->sk_txrehash == SOCK_TXREHASH_ENABLED) {
		sk_set_txhash(sk);
		return true;
	}
	return false;
}

static inline struct dst_entry *
__sk_dst_get(struct sock *sk)
{
	return rcu_dereference_check(sk->sk_dst_cache,
				     lockdep_sock_is_held(sk));
}

static inline struct dst_entry *
sk_dst_get(struct sock *sk)
{
	struct dst_entry *dst;

	rcu_read_lock();
	dst = rcu_dereference(sk->sk_dst_cache);
	if (dst && !atomic_inc_not_zero(&dst->__refcnt))
		dst = NULL;
	rcu_read_unlock();
	return dst;
}

static inline void __dst_negative_advice(struct sock *sk)
{
	struct dst_entry *ndst, *dst = __sk_dst_get(sk);

	if (dst && dst->ops->negative_advice) {
		ndst = dst->ops->negative_advice(dst);

		if (ndst != dst) {
			rcu_assign_pointer(sk->sk_dst_cache, ndst);
			sk_tx_queue_clear(sk);
			sk->sk_dst_pending_confirm = 0;
		}
	}
}

static inline void dst_negative_advice(struct sock *sk)
{
	sk_rethink_txhash(sk);
	__dst_negative_advice(sk);
}

static inline void
__sk_dst_set(struct sock *sk, struct dst_entry *dst)
{
	struct dst_entry *old_dst;

	sk_tx_queue_clear(sk);
	sk->sk_dst_pending_confirm = 0;
	old_dst = rcu_dereference_protected(sk->sk_dst_cache,
					    lockdep_sock_is_held(sk));
	rcu_assign_pointer(sk->sk_dst_cache, dst);
	dst_release(old_dst);
}

static inline void
sk_dst_set(struct sock *sk, struct dst_entry *dst)
{
	struct dst_entry *old_dst;

	sk_tx_queue_clear(sk);
	sk->sk_dst_pending_confirm = 0;
	old_dst = xchg((__force struct dst_entry **)&sk->sk_dst_cache, dst);
	dst_release(old_dst);
}

static inline void
__sk_dst_reset(struct sock *sk)
{
	__sk_dst_set(sk, NULL);
}

static inline void
sk_dst_reset(struct sock *sk)
{
	sk_dst_set(sk, NULL);
}

struct dst_entry *__sk_dst_check(struct sock *sk, u32 cookie);

struct dst_entry *sk_dst_check(struct sock *sk, u32 cookie);

static inline void sk_dst_confirm(struct sock *sk)
{
	if (!READ_ONCE(sk->sk_dst_pending_confirm))
		WRITE_ONCE(sk->sk_dst_pending_confirm, 1);
}

static inline void sock_confirm_neigh(struct sk_buff *skb, struct neighbour *n)
{
	if (skb_get_dst_pending_confirm(skb)) {
		struct sock *sk = skb->sk;

		if (sk && READ_ONCE(sk->sk_dst_pending_confirm))
			WRITE_ONCE(sk->sk_dst_pending_confirm, 0);
		neigh_confirm(n);
	}
}

bool sk_mc_loop(struct sock *sk);

static inline bool sk_can_gso(const struct sock *sk)
{
	return net_gso_ok(sk->sk_route_caps, sk->sk_gso_type);
}

void sk_setup_caps(struct sock *sk, struct dst_entry *dst);

static inline void sk_gso_disable(struct sock *sk)
{
	sk->sk_gso_disabled = 1;
	sk->sk_route_caps &= ~NETIF_F_GSO_MASK;
}

static inline int skb_do_copy_data_nocache(struct sock *sk, struct sk_buff *skb,
					   struct iov_iter *from, char *to,
					   int copy, int offset)
{
	if (skb->ip_summed == CHECKSUM_NONE) {
		__wsum csum = 0;
		if (!csum_and_copy_from_iter_full(to, copy, &csum, from))
			return -EFAULT;
		skb->csum = csum_block_add(skb->csum, csum, offset);
	} else if (sk->sk_route_caps & NETIF_F_NOCACHE_COPY) {
		if (!copy_from_iter_full_nocache(to, copy, from))
			return -EFAULT;
	} else if (!copy_from_iter_full(to, copy, from))
		return -EFAULT;

	return 0;
}

static inline int skb_add_data_nocache(struct sock *sk, struct sk_buff *skb,
				       struct iov_iter *from, int copy)
{
	int err, offset = skb->len;

	err = skb_do_copy_data_nocache(sk, skb, from, skb_put(skb, copy),
				       copy, offset);
	if (err)
		__skb_trim(skb, offset);

	return err;
}

static inline int skb_copy_to_page_nocache(struct sock *sk, struct iov_iter *from,
					   struct sk_buff *skb,
					   struct page *page,
					   int off, int copy)
{
	int err;

	err = skb_do_copy_data_nocache(sk, skb, from, page_address(page) + off,
				       copy, skb->len);
	if (err)
		return err;

	skb_len_add(skb, copy);
	sk_wmem_queued_add(sk, copy);
	sk_mem_charge(sk, copy);
	return 0;
}


static inline int sk_wmem_alloc_get(const struct sock *sk)
{
	return refcount_read(&sk->sk_wmem_alloc) - 1;
}


static inline int sk_rmem_alloc_get(const struct sock *sk)
{
	return atomic_read(&sk->sk_rmem_alloc);
}


static inline bool sk_has_allocations(const struct sock *sk)
{
	return sk_wmem_alloc_get(sk) || sk_rmem_alloc_get(sk);
}


static inline bool skwq_has_sleeper(struct socket_wq *wq)
{
	return wq && wq_has_sleeper(&wq->wait);
}


static inline void sock_poll_wait(struct file *filp, struct socket *sock,
				  poll_table *p)
{
	if (!poll_does_not_wait(p)) {
		poll_wait(filp, &sock->wq.wait, p);
		
		smp_mb();
	}
}

static inline void skb_set_hash_from_sk(struct sk_buff *skb, struct sock *sk)
{
	
	u32 txhash = READ_ONCE(sk->sk_txhash);

	if (txhash) {
		skb->l4_hash = 1;
		skb->hash = txhash;
	}
}

void skb_set_owner_w(struct sk_buff *skb, struct sock *sk);


static inline void skb_set_owner_r(struct sk_buff *skb, struct sock *sk)
{
	skb_orphan(skb);
	skb->sk = sk;
	skb->destructor = sock_rfree;
	atomic_add(skb->truesize, &sk->sk_rmem_alloc);
	sk_mem_charge(sk, skb->truesize);
}

static inline __must_check bool skb_set_owner_sk_safe(struct sk_buff *skb, struct sock *sk)
{
	if (sk && refcount_inc_not_zero(&sk->sk_refcnt)) {
		skb_orphan(skb);
		skb->destructor = sock_efree;
		skb->sk = sk;
		return true;
	}
	return false;
}

static inline void skb_prepare_for_gro(struct sk_buff *skb)
{
	if (skb->destructor != sock_wfree) {
		skb_orphan(skb);
		return;
	}
	skb->slow_gro = 1;
}

void sk_reset_timer(struct sock *sk, struct timer_list *timer,
		    unsigned long expires);

void sk_stop_timer(struct sock *sk, struct timer_list *timer);

void sk_stop_timer_sync(struct sock *sk, struct timer_list *timer);

int __sk_queue_drop_skb(struct sock *sk, struct sk_buff_head *sk_queue,
			struct sk_buff *skb, unsigned int flags,
			void (*destructor)(struct sock *sk,
					   struct sk_buff *skb));
int __sock_queue_rcv_skb(struct sock *sk, struct sk_buff *skb);

int sock_queue_rcv_skb_reason(struct sock *sk, struct sk_buff *skb,
			      enum skb_drop_reason *reason);

static inline int sock_queue_rcv_skb(struct sock *sk, struct sk_buff *skb)
{
	return sock_queue_rcv_skb_reason(sk, skb, NULL);
}

int sock_queue_err_skb(struct sock *sk, struct sk_buff *skb);
struct sk_buff *sock_dequeue_err_skb(struct sock *sk);



static inline int sock_error(struct sock *sk)
{
	int err;

	
	if (likely(data_race(!sk->sk_err)))
		return 0;

	err = xchg(&sk->sk_err, 0);
	return -err;
}

void sk_error_report(struct sock *sk);

static inline unsigned long sock_wspace(struct sock *sk)
{
	int amt = 0;

	if (!(sk->sk_shutdown & SEND_SHUTDOWN)) {
		amt = sk->sk_sndbuf - refcount_read(&sk->sk_wmem_alloc);
		if (amt < 0)
			amt = 0;
	}
	return amt;
}


static inline void sk_set_bit(int nr, struct sock *sk)
{
	if ((nr == SOCKWQ_ASYNC_NOSPACE || nr == SOCKWQ_ASYNC_WAITDATA) &&
	    !sock_flag(sk, SOCK_FASYNC))
		return;

	set_bit(nr, &sk->sk_wq_raw->flags);
}

static inline void sk_clear_bit(int nr, struct sock *sk)
{
	if ((nr == SOCKWQ_ASYNC_NOSPACE || nr == SOCKWQ_ASYNC_WAITDATA) &&
	    !sock_flag(sk, SOCK_FASYNC))
		return;

	clear_bit(nr, &sk->sk_wq_raw->flags);
}

static inline void sk_wake_async(const struct sock *sk, int how, int band)
{
	if (sock_flag(sk, SOCK_FASYNC)) {
		rcu_read_lock();
		sock_wake_async(rcu_dereference(sk->sk_wq), how, band);
		rcu_read_unlock();
	}
}


#define TCP_SKB_MIN_TRUESIZE	(2048 + SKB_DATA_ALIGN(sizeof(struct sk_buff)))

#define SOCK_MIN_SNDBUF		(TCP_SKB_MIN_TRUESIZE * 2)
#define SOCK_MIN_RCVBUF		 TCP_SKB_MIN_TRUESIZE

static inline void sk_stream_moderate_sndbuf(struct sock *sk)
{
	u32 val;

	if (sk->sk_userlocks & SOCK_SNDBUF_LOCK)
		return;

	val = min(sk->sk_sndbuf, sk->sk_wmem_queued >> 1);
	val = max_t(u32, val, sk_unused_reserved_mem(sk));

	WRITE_ONCE(sk->sk_sndbuf, max_t(u32, val, SOCK_MIN_SNDBUF));
}


static inline struct page_frag *sk_page_frag(struct sock *sk)
{
	if ((sk->sk_allocation & (__GFP_DIRECT_RECLAIM | __GFP_MEMALLOC | __GFP_FS)) ==
	    (__GFP_DIRECT_RECLAIM | __GFP_FS))
		return &current->task_frag;

	return &sk->sk_frag;
}

bool sk_page_frag_refill(struct sock *sk, struct page_frag *pfrag);


static inline bool sock_writeable(const struct sock *sk)
{
	return refcount_read(&sk->sk_wmem_alloc) < (READ_ONCE(sk->sk_sndbuf) >> 1);
}

static inline gfp_t gfp_any(void)
{
	return in_softirq() ? GFP_ATOMIC : GFP_KERNEL;
}

static inline gfp_t gfp_memcg_charge(void)
{
	return in_softirq() ? GFP_ATOMIC : GFP_KERNEL;
}

static inline long sock_rcvtimeo(const struct sock *sk, bool noblock)
{
	return noblock ? 0 : sk->sk_rcvtimeo;
}

static inline long sock_sndtimeo(const struct sock *sk, bool noblock)
{
	return noblock ? 0 : sk->sk_sndtimeo;
}

static inline int sock_rcvlowat(const struct sock *sk, int waitall, int len)
{
	int v = waitall ? len : min_t(int, READ_ONCE(sk->sk_rcvlowat), len);

	return v ?: 1;
}


static inline int sock_intr_errno(long timeo)
{
	return timeo == MAX_SCHEDULE_TIMEOUT ? -ERESTARTSYS : -EINTR;
}

struct sock_skb_cb {
	u32 dropcount;
};


#define SOCK_SKB_CB_OFFSET ((sizeof_field(struct sk_buff, cb) - \
			    sizeof(struct sock_skb_cb)))

#define SOCK_SKB_CB(__skb) ((struct sock_skb_cb *)((__skb)->cb + \
			    SOCK_SKB_CB_OFFSET))

#define sock_skb_cb_check_size(size) \
	BUILD_BUG_ON((size) > SOCK_SKB_CB_OFFSET)

static inline void
sock_skb_set_dropcount(const struct sock *sk, struct sk_buff *skb)
{
	SOCK_SKB_CB(skb)->dropcount = sock_flag(sk, SOCK_RXQ_OVFL) ?
						atomic_read(&sk->sk_drops) : 0;
}

static inline void sk_drops_add(struct sock *sk, const struct sk_buff *skb)
{
	int segs = max_t(u16, 1, skb_shinfo(skb)->gso_segs);

	atomic_add(segs, &sk->sk_drops);
}

static inline ktime_t sock_read_timestamp(struct sock *sk)
{
#if BITS_PER_LONG==32
	unsigned int seq;
	ktime_t kt;

	do {
		seq = read_seqbegin(&sk->sk_stamp_seq);
		kt = sk->sk_stamp;
	} while (read_seqretry(&sk->sk_stamp_seq, seq));

	return kt;
#else
	return READ_ONCE(sk->sk_stamp);
#endif
}

static inline void sock_write_timestamp(struct sock *sk, ktime_t kt)
{
#if BITS_PER_LONG==32
	write_seqlock(&sk->sk_stamp_seq);
	sk->sk_stamp = kt;
	write_sequnlock(&sk->sk_stamp_seq);
#else
	WRITE_ONCE(sk->sk_stamp, kt);
#endif
}

void __sock_recv_timestamp(struct msghdr *msg, struct sock *sk,
			   struct sk_buff *skb);
void __sock_recv_wifi_status(struct msghdr *msg, struct sock *sk,
			     struct sk_buff *skb);

static inline void
sock_recv_timestamp(struct msghdr *msg, struct sock *sk, struct sk_buff *skb)
{
	ktime_t kt = skb->tstamp;
	struct skb_shared_hwtstamps *hwtstamps = skb_hwtstamps(skb);

	
	if (sock_flag(sk, SOCK_RCVTSTAMP) ||
	    (sk->sk_tsflags & SOF_TIMESTAMPING_RX_SOFTWARE) ||
	    (kt && sk->sk_tsflags & SOF_TIMESTAMPING_SOFTWARE) ||
	    (hwtstamps->hwtstamp &&
	     (sk->sk_tsflags & SOF_TIMESTAMPING_RAW_HARDWARE)))
		__sock_recv_timestamp(msg, sk, skb);
	else
		sock_write_timestamp(sk, kt);

	if (sock_flag(sk, SOCK_WIFI_STATUS) && skb->wifi_acked_valid)
		__sock_recv_wifi_status(msg, sk, skb);
}

void __sock_recv_cmsgs(struct msghdr *msg, struct sock *sk,
		       struct sk_buff *skb);

#define SK_DEFAULT_STAMP (-1L * NSEC_PER_SEC)
static inline void sock_recv_cmsgs(struct msghdr *msg, struct sock *sk,
				   struct sk_buff *skb)
{
#define FLAGS_RECV_CMSGS ((1UL << SOCK_RXQ_OVFL)			| \
			   (1UL << SOCK_RCVTSTAMP)			| \
			   (1UL << SOCK_RCVMARK))
#define TSFLAGS_ANY	  (SOF_TIMESTAMPING_SOFTWARE			| \
			   SOF_TIMESTAMPING_RAW_HARDWARE)

	if (sk->sk_flags & FLAGS_RECV_CMSGS || sk->sk_tsflags & TSFLAGS_ANY)
		__sock_recv_cmsgs(msg, sk, skb);
	else if (unlikely(sock_flag(sk, SOCK_TIMESTAMP)))
		sock_write_timestamp(sk, skb->tstamp);
	else if (unlikely(sk->sk_stamp == SK_DEFAULT_STAMP))
		sock_write_timestamp(sk, 0);
}

void __sock_tx_timestamp(__u16 tsflags, __u8 *tx_flags);


static inline void _sock_tx_timestamp(struct sock *sk, __u16 tsflags,
				      __u8 *tx_flags, __u32 *tskey)
{
	if (unlikely(tsflags)) {
		__sock_tx_timestamp(tsflags, tx_flags);
		if (tsflags & SOF_TIMESTAMPING_OPT_ID && tskey &&
		    tsflags & SOF_TIMESTAMPING_TX_RECORD_MASK)
			*tskey = atomic_inc_return(&sk->sk_tskey) - 1;
	}
	if (unlikely(sock_flag(sk, SOCK_WIFI_STATUS)))
		*tx_flags |= SKBTX_WIFI_STATUS;
}

static inline void sock_tx_timestamp(struct sock *sk, __u16 tsflags,
				     __u8 *tx_flags)
{
	_sock_tx_timestamp(sk, tsflags, tx_flags, NULL);
}

static inline void skb_setup_tx_timestamp(struct sk_buff *skb, __u16 tsflags)
{
	_sock_tx_timestamp(skb->sk, tsflags, &skb_shinfo(skb)->tx_flags,
			   &skb_shinfo(skb)->tskey);
}

static inline bool sk_is_tcp(const struct sock *sk)
{
	return sk->sk_type == SOCK_STREAM && sk->sk_protocol == IPPROTO_TCP;
}


static inline void sk_eat_skb(struct sock *sk, struct sk_buff *skb)
{
	__skb_unlink(skb, &sk->sk_receive_queue);
	__kfree_skb(skb);
}

static inline bool
skb_sk_is_prefetched(struct sk_buff *skb)
{
#ifdef CONFIG_INET
	return skb->destructor == sock_pfree;
#else
	return false;
#endif 
}


static inline bool sk_fullsock(const struct sock *sk)
{
	return (1 << sk->sk_state) & ~(TCPF_TIME_WAIT | TCPF_NEW_SYN_RECV);
}

static inline bool
sk_is_refcounted(struct sock *sk)
{
	
	return !sk_fullsock(sk) || !sock_flag(sk, SOCK_RCU_FREE);
}


static inline struct sock *
skb_steal_sock(struct sk_buff *skb, bool *refcounted)
{
	if (skb->sk) {
		struct sock *sk = skb->sk;

		*refcounted = true;
		if (skb_sk_is_prefetched(skb))
			*refcounted = sk_is_refcounted(sk);
		skb->destructor = NULL;
		skb->sk = NULL;
		return sk;
	}
	*refcounted = false;
	return NULL;
}


static inline struct sk_buff *sk_validate_xmit_skb(struct sk_buff *skb,
						   struct net_device *dev)
{
#ifdef CONFIG_SOCK_VALIDATE_XMIT
	struct sock *sk = skb->sk;

	if (sk && sk_fullsock(sk) && sk->sk_validate_xmit_skb) {
		skb = sk->sk_validate_xmit_skb(sk, dev, skb);
#ifdef CONFIG_TLS_DEVICE
	} else if (unlikely(skb->decrypted)) {
		pr_warn_ratelimited("unencrypted skb with no associated socket - dropping\n");
		kfree_skb(skb);
		skb = NULL;
#endif
	}
#endif

	return skb;
}


static inline bool sk_listener(const struct sock *sk)
{
	return (1 << sk->sk_state) & (TCPF_LISTEN | TCPF_NEW_SYN_RECV);
}

void sock_enable_timestamp(struct sock *sk, enum sock_flags flag);
int sock_recv_errqueue(struct sock *sk, struct msghdr *msg, int len, int level,
		       int type);

bool sk_ns_capable(const struct sock *sk,
		   struct user_namespace *user_ns, int cap);
bool sk_capable(const struct sock *sk, int cap);
bool sk_net_capable(const struct sock *sk, int cap);

void sk_get_meminfo(const struct sock *sk, u32 *meminfo);


#define _SK_MEM_PACKETS		256
#define _SK_MEM_OVERHEAD	SKB_TRUESIZE(256)
#define SK_WMEM_MAX		(_SK_MEM_OVERHEAD * _SK_MEM_PACKETS)
#define SK_RMEM_MAX		(_SK_MEM_OVERHEAD * _SK_MEM_PACKETS)

extern __u32 sysctl_wmem_max;
extern __u32 sysctl_rmem_max;

extern int sysctl_tstamp_allow_data;
extern int sysctl_optmem_max;

extern __u32 sysctl_wmem_default;
extern __u32 sysctl_rmem_default;

#define SKB_FRAG_PAGE_ORDER	get_order(32768)
DECLARE_STATIC_KEY_FALSE(net_high_order_alloc_disable_key);

static inline int sk_get_wmem0(const struct sock *sk, const struct proto *proto)
{
	
	if (proto->sysctl_wmem_offset)
		return READ_ONCE(*(int *)((void *)sock_net(sk) + proto->sysctl_wmem_offset));

	return READ_ONCE(*proto->sysctl_wmem);
}

static inline int sk_get_rmem0(const struct sock *sk, const struct proto *proto)
{
	
	if (proto->sysctl_rmem_offset)
		return READ_ONCE(*(int *)((void *)sock_net(sk) + proto->sysctl_rmem_offset));

	return READ_ONCE(*proto->sysctl_rmem);
}


static inline void sk_pacing_shift_update(struct sock *sk, int val)
{
	if (!sk || !sk_fullsock(sk) || READ_ONCE(sk->sk_pacing_shift) == val)
		return;
	WRITE_ONCE(sk->sk_pacing_shift, val);
}


static inline bool sk_dev_equal_l3scope(struct sock *sk, int dif)
{
	int bound_dev_if = READ_ONCE(sk->sk_bound_dev_if);
	int mdif;

	if (!bound_dev_if || bound_dev_if == dif)
		return true;

	mdif = l3mdev_master_ifindex_by_index(sock_net(sk), dif);
	if (mdif && mdif == bound_dev_if)
		return true;

	return false;
}

void sock_def_readable(struct sock *sk);

int sock_bindtoindex(struct sock *sk, int ifindex, bool lock_sk);
void sock_set_timestamp(struct sock *sk, int optname, bool valbool);
int sock_set_timestamping(struct sock *sk, int optname,
			  struct so_timestamping timestamping);

void sock_enable_timestamps(struct sock *sk);
void sock_no_linger(struct sock *sk);
void sock_set_keepalive(struct sock *sk);
void sock_set_priority(struct sock *sk, u32 priority);
void sock_set_rcvbuf(struct sock *sk, int val);
void sock_set_mark(struct sock *sk, u32 val);
void sock_set_reuseaddr(struct sock *sk);
void sock_set_reuseport(struct sock *sk);
void sock_set_sndtimeo(struct sock *sk, s64 secs);

int sock_bind_add(struct sock *sk, struct sockaddr *addr, int addr_len);

int sock_get_timeout(long timeo, void *optval, bool old_timeval);
int sock_copy_user_timeval(struct __kernel_sock_timeval *tv,
			   sockptr_t optval, int optlen, bool old_timeval);

static inline bool sk_is_readable(struct sock *sk)
{
	if (sk->sk_prot->sock_is_readable)
		return sk->sk_prot->sock_is_readable(sk);
	return false;
}
#endif	
