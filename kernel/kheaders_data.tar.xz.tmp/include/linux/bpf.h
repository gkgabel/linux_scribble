/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _LINUX_BPF_H
#define _LINUX_BPF_H 1

#include <uapi/linux/bpf.h>
#include <uapi/linux/filter.h>

#include <linux/workqueue.h>
#include <linux/file.h>
#include <linux/percpu.h>
#include <linux/err.h>
#include <linux/rbtree_latch.h>
#include <linux/numa.h>
#include <linux/mm_types.h>
#include <linux/wait.h>
#include <linux/refcount.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/capability.h>
#include <linux/sched/mm.h>
#include <linux/slab.h>
#include <linux/percpu-refcount.h>
#include <linux/stddef.h>
#include <linux/bpfptr.h>
#include <linux/btf.h>
#include <linux/rcupdate_trace.h>
#include <linux/static_call.h>

struct bpf_verifier_env;
struct bpf_verifier_log;
struct perf_event;
struct bpf_prog;
struct bpf_prog_aux;
struct bpf_map;
struct sock;
struct seq_file;
struct btf;
struct btf_type;
struct exception_table_entry;
struct seq_operations;
struct bpf_iter_aux_info;
struct bpf_local_storage;
struct bpf_local_storage_map;
struct kobject;
struct mem_cgroup;
struct module;
struct bpf_func_state;
struct ftrace_ops;

extern struct idr btf_idr;
extern spinlock_t btf_idr_lock;
extern struct kobject *btf_kobj;

typedef u64 (*bpf_callback_t)(u64, u64, u64, u64, u64);
typedef int (*bpf_iter_init_seq_priv_t)(void *private_data,
					struct bpf_iter_aux_info *aux);
typedef void (*bpf_iter_fini_seq_priv_t)(void *private_data);
typedef unsigned int (*bpf_func_t)(const void *,
				   const struct bpf_insn *);
struct bpf_iter_seq_info {
	const struct seq_operations *seq_ops;
	bpf_iter_init_seq_priv_t init_seq_private;
	bpf_iter_fini_seq_priv_t fini_seq_private;
	u32 seq_priv_size;
};


struct bpf_map_ops {
	
	int (*map_alloc_check)(union bpf_attr *attr);
	struct bpf_map *(*map_alloc)(union bpf_attr *attr);
	void (*map_release)(struct bpf_map *map, struct file *map_file);
	void (*map_free)(struct bpf_map *map);
	int (*map_get_next_key)(struct bpf_map *map, void *key, void *next_key);
	void (*map_release_uref)(struct bpf_map *map);
	void *(*map_lookup_elem_sys_only)(struct bpf_map *map, void *key);
	int (*map_lookup_batch)(struct bpf_map *map, const union bpf_attr *attr,
				union bpf_attr __user *uattr);
	int (*map_lookup_and_delete_elem)(struct bpf_map *map, void *key,
					  void *value, u64 flags);
	int (*map_lookup_and_delete_batch)(struct bpf_map *map,
					   const union bpf_attr *attr,
					   union bpf_attr __user *uattr);
	int (*map_update_batch)(struct bpf_map *map, const union bpf_attr *attr,
				union bpf_attr __user *uattr);
	int (*map_delete_batch)(struct bpf_map *map, const union bpf_attr *attr,
				union bpf_attr __user *uattr);

	
	void *(*map_lookup_elem)(struct bpf_map *map, void *key);
	int (*map_update_elem)(struct bpf_map *map, void *key, void *value, u64 flags);
	int (*map_delete_elem)(struct bpf_map *map, void *key);
	int (*map_push_elem)(struct bpf_map *map, void *value, u64 flags);
	int (*map_pop_elem)(struct bpf_map *map, void *value);
	int (*map_peek_elem)(struct bpf_map *map, void *value);
	void *(*map_lookup_percpu_elem)(struct bpf_map *map, void *key, u32 cpu);

	
	void *(*map_fd_get_ptr)(struct bpf_map *map, struct file *map_file,
				int fd);
	void (*map_fd_put_ptr)(void *ptr);
	int (*map_gen_lookup)(struct bpf_map *map, struct bpf_insn *insn_buf);
	u32 (*map_fd_sys_lookup_elem)(void *ptr);
	void (*map_seq_show_elem)(struct bpf_map *map, void *key,
				  struct seq_file *m);
	int (*map_check_btf)(const struct bpf_map *map,
			     const struct btf *btf,
			     const struct btf_type *key_type,
			     const struct btf_type *value_type);

	
	int (*map_poke_track)(struct bpf_map *map, struct bpf_prog_aux *aux);
	void (*map_poke_untrack)(struct bpf_map *map, struct bpf_prog_aux *aux);
	void (*map_poke_run)(struct bpf_map *map, u32 key, struct bpf_prog *old,
			     struct bpf_prog *new);

	
	int (*map_direct_value_addr)(const struct bpf_map *map,
				     u64 *imm, u32 off);
	int (*map_direct_value_meta)(const struct bpf_map *map,
				     u64 imm, u32 *off);
	int (*map_mmap)(struct bpf_map *map, struct vm_area_struct *vma);
	__poll_t (*map_poll)(struct bpf_map *map, struct file *filp,
			     struct poll_table_struct *pts);

	
	int (*map_local_storage_charge)(struct bpf_local_storage_map *smap,
					void *owner, u32 size);
	void (*map_local_storage_uncharge)(struct bpf_local_storage_map *smap,
					   void *owner, u32 size);
	struct bpf_local_storage __rcu ** (*map_owner_storage_ptr)(void *owner);

	
	int (*map_redirect)(struct bpf_map *map, u32 ifindex, u64 flags);

	
	bool (*map_meta_equal)(const struct bpf_map *meta0,
			       const struct bpf_map *meta1);


	int (*map_set_for_each_callback_args)(struct bpf_verifier_env *env,
					      struct bpf_func_state *caller,
					      struct bpf_func_state *callee);
	int (*map_for_each_callback)(struct bpf_map *map,
				     bpf_callback_t callback_fn,
				     void *callback_ctx, u64 flags);

	
	int *map_btf_id;

	
	const struct bpf_iter_seq_info *iter_seq_info;
};

enum {
	
	BPF_MAP_VALUE_OFF_MAX = 8,
	BPF_MAP_OFF_ARR_MAX   = BPF_MAP_VALUE_OFF_MAX +
				1 + 
				1,  
};

enum bpf_kptr_type {
	BPF_KPTR_UNREF,
	BPF_KPTR_REF,
};

struct bpf_map_value_off_desc {
	u32 offset;
	enum bpf_kptr_type type;
	struct {
		struct btf *btf;
		struct module *module;
		btf_dtor_kfunc_t dtor;
		u32 btf_id;
	} kptr;
};

struct bpf_map_value_off {
	u32 nr_off;
	struct bpf_map_value_off_desc off[];
};

struct bpf_map_off_arr {
	u32 cnt;
	u32 field_off[BPF_MAP_OFF_ARR_MAX];
	u8 field_sz[BPF_MAP_OFF_ARR_MAX];
};

struct bpf_map {
	
	const struct bpf_map_ops *ops ____cacheline_aligned;
	struct bpf_map *inner_map_meta;
#ifdef CONFIG_SECURITY
	void *security;
#endif
	enum bpf_map_type map_type;
	u32 key_size;
	u32 value_size;
	u32 max_entries;
	u64 map_extra; 
	u32 map_flags;
	int spin_lock_off; 
	struct bpf_map_value_off *kptr_off_tab;
	int timer_off; 
	u32 id;
	int numa_node;
	u32 btf_key_type_id;
	u32 btf_value_type_id;
	u32 btf_vmlinux_value_type_id;
	struct btf *btf;
#ifdef CONFIG_MEMCG_KMEM
	struct obj_cgroup *objcg;
#endif
	char name[BPF_OBJ_NAME_LEN];
	struct bpf_map_off_arr *off_arr;
	
	atomic64_t refcnt ____cacheline_aligned;
	atomic64_t usercnt;
	struct work_struct work;
	struct mutex freeze_mutex;
	atomic64_t writecnt;
	
	struct {
		spinlock_t lock;
		enum bpf_prog_type type;
		bool jited;
		bool xdp_has_frags;
	} owner;
	bool bypass_spec_v1;
	bool frozen; 
};

static inline bool map_value_has_spin_lock(const struct bpf_map *map)
{
	return map->spin_lock_off >= 0;
}

static inline bool map_value_has_timer(const struct bpf_map *map)
{
	return map->timer_off >= 0;
}

static inline bool map_value_has_kptrs(const struct bpf_map *map)
{
	return !IS_ERR_OR_NULL(map->kptr_off_tab);
}

static inline void check_and_init_map_value(struct bpf_map *map, void *dst)
{
	if (unlikely(map_value_has_spin_lock(map)))
		memset(dst + map->spin_lock_off, 0, sizeof(struct bpf_spin_lock));
	if (unlikely(map_value_has_timer(map)))
		memset(dst + map->timer_off, 0, sizeof(struct bpf_timer));
	if (unlikely(map_value_has_kptrs(map))) {
		struct bpf_map_value_off *tab = map->kptr_off_tab;
		int i;

		for (i = 0; i < tab->nr_off; i++)
			*(u64 *)(dst + tab->off[i].offset) = 0;
	}
}


static inline void copy_map_value(struct bpf_map *map, void *dst, void *src)
{
	u32 curr_off = 0;
	int i;

	if (likely(!map->off_arr)) {
		memcpy(dst, src, map->value_size);
		return;
	}

	for (i = 0; i < map->off_arr->cnt; i++) {
		u32 next_off = map->off_arr->field_off[i];

		memcpy(dst + curr_off, src + curr_off, next_off - curr_off);
		curr_off += map->off_arr->field_sz[i];
	}
	memcpy(dst + curr_off, src + curr_off, map->value_size - curr_off);
}
void copy_map_value_locked(struct bpf_map *map, void *dst, void *src,
			   bool lock_src);
void bpf_timer_cancel_and_free(void *timer);
int bpf_obj_name_cpy(char *dst, const char *src, unsigned int size);

struct bpf_offload_dev;
struct bpf_offloaded_map;

struct bpf_map_dev_ops {
	int (*map_get_next_key)(struct bpf_offloaded_map *map,
				void *key, void *next_key);
	int (*map_lookup_elem)(struct bpf_offloaded_map *map,
			       void *key, void *value);
	int (*map_update_elem)(struct bpf_offloaded_map *map,
			       void *key, void *value, u64 flags);
	int (*map_delete_elem)(struct bpf_offloaded_map *map, void *key);
};

struct bpf_offloaded_map {
	struct bpf_map map;
	struct net_device *netdev;
	const struct bpf_map_dev_ops *dev_ops;
	void *dev_priv;
	struct list_head offloads;
};

static inline struct bpf_offloaded_map *map_to_offmap(struct bpf_map *map)
{
	return container_of(map, struct bpf_offloaded_map, map);
}

static inline bool bpf_map_offload_neutral(const struct bpf_map *map)
{
	return map->map_type == BPF_MAP_TYPE_PERF_EVENT_ARRAY;
}

static inline bool bpf_map_support_seq_show(const struct bpf_map *map)
{
	return (map->btf_value_type_id || map->btf_vmlinux_value_type_id) &&
		map->ops->map_seq_show_elem;
}

int map_check_no_btf(const struct bpf_map *map,
		     const struct btf *btf,
		     const struct btf_type *key_type,
		     const struct btf_type *value_type);

bool bpf_map_meta_equal(const struct bpf_map *meta0,
			const struct bpf_map *meta1);

extern const struct bpf_map_ops bpf_map_offload_ops;


#define BPF_BASE_TYPE_BITS	8

enum bpf_type_flag {
	
	PTR_MAYBE_NULL		= BIT(0 + BPF_BASE_TYPE_BITS),

	
	MEM_RDONLY		= BIT(1 + BPF_BASE_TYPE_BITS),

	
	MEM_ALLOC		= BIT(2 + BPF_BASE_TYPE_BITS),

	
	MEM_USER		= BIT(3 + BPF_BASE_TYPE_BITS),

	
	MEM_PERCPU		= BIT(4 + BPF_BASE_TYPE_BITS),

	
	OBJ_RELEASE		= BIT(5 + BPF_BASE_TYPE_BITS),

	
	PTR_UNTRUSTED		= BIT(6 + BPF_BASE_TYPE_BITS),

	MEM_UNINIT		= BIT(7 + BPF_BASE_TYPE_BITS),

	
	DYNPTR_TYPE_LOCAL	= BIT(8 + BPF_BASE_TYPE_BITS),

	
	DYNPTR_TYPE_RINGBUF	= BIT(9 + BPF_BASE_TYPE_BITS),

	
	MEM_FIXED_SIZE		= BIT(10 + BPF_BASE_TYPE_BITS),

	__BPF_TYPE_FLAG_MAX,
	__BPF_TYPE_LAST_FLAG	= __BPF_TYPE_FLAG_MAX - 1,
};

#define DYNPTR_TYPE_FLAG_MASK	(DYNPTR_TYPE_LOCAL | DYNPTR_TYPE_RINGBUF)


#define BPF_BASE_TYPE_LIMIT	(1UL << BPF_BASE_TYPE_BITS)


#define BPF_TYPE_LIMIT		(__BPF_TYPE_LAST_FLAG | (__BPF_TYPE_LAST_FLAG - 1))


enum bpf_arg_type {
	ARG_DONTCARE = 0,	

	
	ARG_CONST_MAP_PTR,	
	ARG_PTR_TO_MAP_KEY,	
	ARG_PTR_TO_MAP_VALUE,	

	
	ARG_PTR_TO_MEM,		

	ARG_CONST_SIZE,		
	ARG_CONST_SIZE_OR_ZERO,	

	ARG_PTR_TO_CTX,		
	ARG_ANYTHING,		
	ARG_PTR_TO_SPIN_LOCK,	
	ARG_PTR_TO_SOCK_COMMON,	
	ARG_PTR_TO_INT,		
	ARG_PTR_TO_LONG,	
	ARG_PTR_TO_SOCKET,	
	ARG_PTR_TO_BTF_ID,	
	ARG_PTR_TO_ALLOC_MEM,	
	ARG_CONST_ALLOC_SIZE_OR_ZERO,	
	ARG_PTR_TO_BTF_ID_SOCK_COMMON,	
	ARG_PTR_TO_PERCPU_BTF_ID,	
	ARG_PTR_TO_FUNC,	
	ARG_PTR_TO_STACK,	
	ARG_PTR_TO_CONST_STR,	
	ARG_PTR_TO_TIMER,	
	ARG_PTR_TO_KPTR,	
	ARG_PTR_TO_DYNPTR,      
	__BPF_ARG_TYPE_MAX,

	
	ARG_PTR_TO_MAP_VALUE_OR_NULL	= PTR_MAYBE_NULL | ARG_PTR_TO_MAP_VALUE,
	ARG_PTR_TO_MEM_OR_NULL		= PTR_MAYBE_NULL | ARG_PTR_TO_MEM,
	ARG_PTR_TO_CTX_OR_NULL		= PTR_MAYBE_NULL | ARG_PTR_TO_CTX,
	ARG_PTR_TO_SOCKET_OR_NULL	= PTR_MAYBE_NULL | ARG_PTR_TO_SOCKET,
	ARG_PTR_TO_ALLOC_MEM_OR_NULL	= PTR_MAYBE_NULL | ARG_PTR_TO_ALLOC_MEM,
	ARG_PTR_TO_STACK_OR_NULL	= PTR_MAYBE_NULL | ARG_PTR_TO_STACK,
	ARG_PTR_TO_BTF_ID_OR_NULL	= PTR_MAYBE_NULL | ARG_PTR_TO_BTF_ID,
	
	ARG_PTR_TO_UNINIT_MEM		= MEM_UNINIT | ARG_PTR_TO_MEM,
	
	ARG_PTR_TO_FIXED_SIZE_MEM	= MEM_FIXED_SIZE | ARG_PTR_TO_MEM,

	
	__BPF_ARG_TYPE_LIMIT	= BPF_TYPE_LIMIT,
};
static_assert(__BPF_ARG_TYPE_MAX <= BPF_BASE_TYPE_LIMIT);


enum bpf_return_type {
	RET_INTEGER,			
	RET_VOID,			
	RET_PTR_TO_MAP_VALUE,		
	RET_PTR_TO_SOCKET,		
	RET_PTR_TO_TCP_SOCK,		
	RET_PTR_TO_SOCK_COMMON,		
	RET_PTR_TO_ALLOC_MEM,		
	RET_PTR_TO_MEM_OR_BTF_ID,	
	RET_PTR_TO_BTF_ID,		
	__BPF_RET_TYPE_MAX,

	
	RET_PTR_TO_MAP_VALUE_OR_NULL	= PTR_MAYBE_NULL | RET_PTR_TO_MAP_VALUE,
	RET_PTR_TO_SOCKET_OR_NULL	= PTR_MAYBE_NULL | RET_PTR_TO_SOCKET,
	RET_PTR_TO_TCP_SOCK_OR_NULL	= PTR_MAYBE_NULL | RET_PTR_TO_TCP_SOCK,
	RET_PTR_TO_SOCK_COMMON_OR_NULL	= PTR_MAYBE_NULL | RET_PTR_TO_SOCK_COMMON,
	RET_PTR_TO_ALLOC_MEM_OR_NULL	= PTR_MAYBE_NULL | MEM_ALLOC | RET_PTR_TO_ALLOC_MEM,
	RET_PTR_TO_DYNPTR_MEM_OR_NULL	= PTR_MAYBE_NULL | RET_PTR_TO_ALLOC_MEM,
	RET_PTR_TO_BTF_ID_OR_NULL	= PTR_MAYBE_NULL | RET_PTR_TO_BTF_ID,

	
	__BPF_RET_TYPE_LIMIT	= BPF_TYPE_LIMIT,
};
static_assert(__BPF_RET_TYPE_MAX <= BPF_BASE_TYPE_LIMIT);


struct bpf_func_proto {
	u64 (*func)(u64 r1, u64 r2, u64 r3, u64 r4, u64 r5);
	bool gpl_only;
	bool pkt_access;
	enum bpf_return_type ret_type;
	union {
		struct {
			enum bpf_arg_type arg1_type;
			enum bpf_arg_type arg2_type;
			enum bpf_arg_type arg3_type;
			enum bpf_arg_type arg4_type;
			enum bpf_arg_type arg5_type;
		};
		enum bpf_arg_type arg_type[5];
	};
	union {
		struct {
			u32 *arg1_btf_id;
			u32 *arg2_btf_id;
			u32 *arg3_btf_id;
			u32 *arg4_btf_id;
			u32 *arg5_btf_id;
		};
		u32 *arg_btf_id[5];
		struct {
			size_t arg1_size;
			size_t arg2_size;
			size_t arg3_size;
			size_t arg4_size;
			size_t arg5_size;
		};
		size_t arg_size[5];
	};
	int *ret_btf_id; 
	bool (*allowed)(const struct bpf_prog *prog);
};


struct bpf_context;

enum bpf_access_type {
	BPF_READ = 1,
	BPF_WRITE = 2
};



enum bpf_reg_type {
	NOT_INIT = 0,		 
	SCALAR_VALUE,		 
	PTR_TO_CTX,		 
	CONST_PTR_TO_MAP,	 
	PTR_TO_MAP_VALUE,	 
	PTR_TO_MAP_KEY,		 
	PTR_TO_STACK,		 
	PTR_TO_PACKET_META,	 
	PTR_TO_PACKET,		 
	PTR_TO_PACKET_END,	 
	PTR_TO_FLOW_KEYS,	 
	PTR_TO_SOCKET,		 
	PTR_TO_SOCK_COMMON,	 
	PTR_TO_TCP_SOCK,	 
	PTR_TO_TP_BUFFER,	 
	PTR_TO_XDP_SOCK,	 
	
	PTR_TO_BTF_ID,
	
	PTR_TO_MEM,		 
	PTR_TO_BUF,		 
	PTR_TO_FUNC,		 
	__BPF_REG_TYPE_MAX,

	
	PTR_TO_MAP_VALUE_OR_NULL	= PTR_MAYBE_NULL | PTR_TO_MAP_VALUE,
	PTR_TO_SOCKET_OR_NULL		= PTR_MAYBE_NULL | PTR_TO_SOCKET,
	PTR_TO_SOCK_COMMON_OR_NULL	= PTR_MAYBE_NULL | PTR_TO_SOCK_COMMON,
	PTR_TO_TCP_SOCK_OR_NULL		= PTR_MAYBE_NULL | PTR_TO_TCP_SOCK,
	PTR_TO_BTF_ID_OR_NULL		= PTR_MAYBE_NULL | PTR_TO_BTF_ID,

	
	__BPF_REG_TYPE_LIMIT	= BPF_TYPE_LIMIT,
};
static_assert(__BPF_REG_TYPE_MAX <= BPF_BASE_TYPE_LIMIT);


struct bpf_insn_access_aux {
	enum bpf_reg_type reg_type;
	union {
		int ctx_field_size;
		struct {
			struct btf *btf;
			u32 btf_id;
		};
	};
	struct bpf_verifier_log *log; 
};

static inline void
bpf_ctx_record_field_size(struct bpf_insn_access_aux *aux, u32 size)
{
	aux->ctx_field_size = size;
}

static inline bool bpf_pseudo_func(const struct bpf_insn *insn)
{
	return insn->code == (BPF_LD | BPF_IMM | BPF_DW) &&
	       insn->src_reg == BPF_PSEUDO_FUNC;
}

struct bpf_prog_ops {
	int (*test_run)(struct bpf_prog *prog, const union bpf_attr *kattr,
			union bpf_attr __user *uattr);
};

struct bpf_verifier_ops {
	
	const struct bpf_func_proto *
	(*get_func_proto)(enum bpf_func_id func_id,
			  const struct bpf_prog *prog);

	
	bool (*is_valid_access)(int off, int size, enum bpf_access_type type,
				const struct bpf_prog *prog,
				struct bpf_insn_access_aux *info);
	int (*gen_prologue)(struct bpf_insn *insn, bool direct_write,
			    const struct bpf_prog *prog);
	int (*gen_ld_abs)(const struct bpf_insn *orig,
			  struct bpf_insn *insn_buf);
	u32 (*convert_ctx_access)(enum bpf_access_type type,
				  const struct bpf_insn *src,
				  struct bpf_insn *dst,
				  struct bpf_prog *prog, u32 *target_size);
	int (*btf_struct_access)(struct bpf_verifier_log *log,
				 const struct btf *btf,
				 const struct btf_type *t, int off, int size,
				 enum bpf_access_type atype,
				 u32 *next_btf_id, enum bpf_type_flag *flag);
};

struct bpf_prog_offload_ops {
	
	int (*insn_hook)(struct bpf_verifier_env *env,
			 int insn_idx, int prev_insn_idx);
	int (*finalize)(struct bpf_verifier_env *env);
	
	int (*replace_insn)(struct bpf_verifier_env *env, u32 off,
			    struct bpf_insn *insn);
	int (*remove_insns)(struct bpf_verifier_env *env, u32 off, u32 cnt);
	
	int (*prepare)(struct bpf_prog *prog);
	int (*translate)(struct bpf_prog *prog);
	void (*destroy)(struct bpf_prog *prog);
};

struct bpf_prog_offload {
	struct bpf_prog		*prog;
	struct net_device	*netdev;
	struct bpf_offload_dev	*offdev;
	void			*dev_priv;
	struct list_head	offloads;
	bool			dev_state;
	bool			opt_failed;
	void			*jited_image;
	u32			jited_len;
};

enum bpf_cgroup_storage_type {
	BPF_CGROUP_STORAGE_SHARED,
	BPF_CGROUP_STORAGE_PERCPU,
	__BPF_CGROUP_STORAGE_MAX
};

#define MAX_BPF_CGROUP_STORAGE_TYPE __BPF_CGROUP_STORAGE_MAX


#define MAX_BPF_FUNC_ARGS 12


#define MAX_BPF_FUNC_REG_ARGS 5

struct btf_func_model {
	u8 ret_size;
	u8 nr_args;
	u8 arg_size[MAX_BPF_FUNC_ARGS];
};


#define BPF_TRAMP_F_RESTORE_REGS	BIT(0)

#define BPF_TRAMP_F_CALL_ORIG		BIT(1)

#define BPF_TRAMP_F_SKIP_FRAME		BIT(2)

#define BPF_TRAMP_F_IP_ARG		BIT(3)

#define BPF_TRAMP_F_RET_FENTRY_RET	BIT(4)


#define BPF_TRAMP_F_ORIG_STACK		BIT(5)


#define BPF_TRAMP_F_SHARE_IPMODIFY	BIT(6)


#define BPF_MAX_TRAMP_LINKS 38

struct bpf_tramp_links {
	struct bpf_tramp_link *links[BPF_MAX_TRAMP_LINKS];
	int nr_links;
};

struct bpf_tramp_run_ctx;


struct bpf_tramp_image;
int arch_prepare_bpf_trampoline(struct bpf_tramp_image *tr, void *image, void *image_end,
				const struct btf_func_model *m, u32 flags,
				struct bpf_tramp_links *tlinks,
				void *orig_call);

u64 notrace __bpf_prog_enter(struct bpf_prog *prog, struct bpf_tramp_run_ctx *run_ctx);
void notrace __bpf_prog_exit(struct bpf_prog *prog, u64 start, struct bpf_tramp_run_ctx *run_ctx);
u64 notrace __bpf_prog_enter_sleepable(struct bpf_prog *prog, struct bpf_tramp_run_ctx *run_ctx);
void notrace __bpf_prog_exit_sleepable(struct bpf_prog *prog, u64 start,
				       struct bpf_tramp_run_ctx *run_ctx);
u64 notrace __bpf_prog_enter_lsm_cgroup(struct bpf_prog *prog,
					struct bpf_tramp_run_ctx *run_ctx);
void notrace __bpf_prog_exit_lsm_cgroup(struct bpf_prog *prog, u64 start,
					struct bpf_tramp_run_ctx *run_ctx);
void notrace __bpf_tramp_enter(struct bpf_tramp_image *tr);
void notrace __bpf_tramp_exit(struct bpf_tramp_image *tr);

struct bpf_ksym {
	unsigned long		 start;
	unsigned long		 end;
	char			 name[KSYM_NAME_LEN];
	struct list_head	 lnode;
	struct latch_tree_node	 tnode;
	bool			 prog;
};

enum bpf_tramp_prog_type {
	BPF_TRAMP_FENTRY,
	BPF_TRAMP_FEXIT,
	BPF_TRAMP_MODIFY_RETURN,
	BPF_TRAMP_MAX,
	BPF_TRAMP_REPLACE, 
};

struct bpf_tramp_image {
	void *image;
	struct bpf_ksym ksym;
	struct percpu_ref pcref;
	void *ip_after_call;
	void *ip_epilogue;
	union {
		struct rcu_head rcu;
		struct work_struct work;
	};
};

struct bpf_trampoline {
	
	struct hlist_node hlist;
	struct ftrace_ops *fops;
	
	struct mutex mutex;
	refcount_t refcnt;
	u32 flags;
	u64 key;
	struct {
		struct btf_func_model model;
		void *addr;
		bool ftrace_managed;
	} func;
	
	struct bpf_prog *extension_prog;
	
	struct hlist_head progs_hlist[BPF_TRAMP_MAX];
	
	int progs_cnt[BPF_TRAMP_MAX];
	
	struct bpf_tramp_image *cur_image;
	u64 selector;
	struct module *mod;
};

struct bpf_attach_target_info {
	struct btf_func_model fmodel;
	long tgt_addr;
	const char *tgt_name;
	const struct btf_type *tgt_type;
};

#define BPF_DISPATCHER_MAX 48 

struct bpf_dispatcher_prog {
	struct bpf_prog *prog;
	refcount_t users;
};

struct bpf_dispatcher {
	
	struct mutex mutex;
	void *func;
	struct bpf_dispatcher_prog progs[BPF_DISPATCHER_MAX];
	int num_progs;
	void *image;
	void *rw_image;
	u32 image_off;
	struct bpf_ksym ksym;
#ifdef CONFIG_HAVE_STATIC_CALL
	struct static_call_key *sc_key;
	void *sc_tramp;
#endif
};

static __always_inline __nocfi unsigned int bpf_dispatcher_nop_func(
	const void *ctx,
	const struct bpf_insn *insnsi,
	bpf_func_t bpf_func)
{
	return bpf_func(ctx, insnsi);
}

#ifdef CONFIG_BPF_JIT
int bpf_trampoline_link_prog(struct bpf_tramp_link *link, struct bpf_trampoline *tr);
int bpf_trampoline_unlink_prog(struct bpf_tramp_link *link, struct bpf_trampoline *tr);
struct bpf_trampoline *bpf_trampoline_get(u64 key,
					  struct bpf_attach_target_info *tgt_info);
void bpf_trampoline_put(struct bpf_trampoline *tr);
int arch_prepare_bpf_dispatcher(void *image, void *buf, s64 *funcs, int num_funcs);


#ifdef CONFIG_HAVE_STATIC_CALL

#define __BPF_DISPATCHER_SC_INIT(_name)				\
	.sc_key = &STATIC_CALL_KEY(_name),			\
	.sc_tramp = STATIC_CALL_TRAMP_ADDR(_name),

#define __BPF_DISPATCHER_SC(name)				\
	DEFINE_STATIC_CALL(bpf_dispatcher_##name##_call, bpf_dispatcher_nop_func)

#define __BPF_DISPATCHER_CALL(name)				\
	static_call(bpf_dispatcher_##name##_call)(ctx, insnsi, bpf_func)

#define __BPF_DISPATCHER_UPDATE(_d, _new)			\
	__static_call_update((_d)->sc_key, (_d)->sc_tramp, (_new))

#else
#define __BPF_DISPATCHER_SC_INIT(name)
#define __BPF_DISPATCHER_SC(name)
#define __BPF_DISPATCHER_CALL(name)		bpf_func(ctx, insnsi)
#define __BPF_DISPATCHER_UPDATE(_d, _new)
#endif

#define BPF_DISPATCHER_INIT(_name) {				\
	.mutex = __MUTEX_INITIALIZER(_name.mutex),		\
	.func = &_name##_func,					\
	.progs = {},						\
	.num_progs = 0,						\
	.image = NULL,						\
	.image_off = 0,						\
	.ksym = {						\
		.name  = #_name,				\
		.lnode = LIST_HEAD_INIT(_name.ksym.lnode),	\
	},							\
	__BPF_DISPATCHER_SC_INIT(_name##_call)			\
}

#define DEFINE_BPF_DISPATCHER(name)					\
	__BPF_DISPATCHER_SC(name);					\
	noinline __nocfi unsigned int bpf_dispatcher_##name##_func(	\
		const void *ctx,					\
		const struct bpf_insn *insnsi,				\
		bpf_func_t bpf_func)					\
	{								\
		return __BPF_DISPATCHER_CALL(name);			\
	}								\
	EXPORT_SYMBOL(bpf_dispatcher_##name##_func);			\
	struct bpf_dispatcher bpf_dispatcher_##name =			\
		BPF_DISPATCHER_INIT(bpf_dispatcher_##name);

#define DECLARE_BPF_DISPATCHER(name)					\
	unsigned int bpf_dispatcher_##name##_func(			\
		const void *ctx,					\
		const struct bpf_insn *insnsi,				\
		bpf_func_t bpf_func);					\
	extern struct bpf_dispatcher bpf_dispatcher_##name;

#define BPF_DISPATCHER_FUNC(name) bpf_dispatcher_##name##_func
#define BPF_DISPATCHER_PTR(name) (&bpf_dispatcher_##name)
void bpf_dispatcher_change_prog(struct bpf_dispatcher *d, struct bpf_prog *from,
				struct bpf_prog *to);

void *bpf_jit_alloc_exec_page(void);
void bpf_image_ksym_add(void *data, struct bpf_ksym *ksym);
void bpf_image_ksym_del(struct bpf_ksym *ksym);
void bpf_ksym_add(struct bpf_ksym *ksym);
void bpf_ksym_del(struct bpf_ksym *ksym);
int bpf_jit_charge_modmem(u32 size);
void bpf_jit_uncharge_modmem(u32 size);
bool bpf_prog_has_trampoline(const struct bpf_prog *prog);
#else
static inline int bpf_trampoline_link_prog(struct bpf_tramp_link *link,
					   struct bpf_trampoline *tr)
{
	return -ENOTSUPP;
}
static inline int bpf_trampoline_unlink_prog(struct bpf_tramp_link *link,
					     struct bpf_trampoline *tr)
{
	return -ENOTSUPP;
}
static inline struct bpf_trampoline *bpf_trampoline_get(u64 key,
							struct bpf_attach_target_info *tgt_info)
{
	return ERR_PTR(-EOPNOTSUPP);
}
static inline void bpf_trampoline_put(struct bpf_trampoline *tr) {}
#define DEFINE_BPF_DISPATCHER(name)
#define DECLARE_BPF_DISPATCHER(name)
#define BPF_DISPATCHER_FUNC(name) bpf_dispatcher_nop_func
#define BPF_DISPATCHER_PTR(name) NULL
static inline void bpf_dispatcher_change_prog(struct bpf_dispatcher *d,
					      struct bpf_prog *from,
					      struct bpf_prog *to) {}
static inline bool is_bpf_image_address(unsigned long address)
{
	return false;
}
static inline bool bpf_prog_has_trampoline(const struct bpf_prog *prog)
{
	return false;
}
#endif

struct bpf_func_info_aux {
	u16 linkage;
	bool unreliable;
};

enum bpf_jit_poke_reason {
	BPF_POKE_REASON_TAIL_CALL,
};


struct bpf_jit_poke_descriptor {
	void *tailcall_target;
	void *tailcall_bypass;
	void *bypass_addr;
	void *aux;
	union {
		struct {
			struct bpf_map *map;
			u32 key;
		} tail_call;
	};
	bool tailcall_target_stable;
	u8 adj_off;
	u16 reason;
	u32 insn_idx;
};


struct bpf_ctx_arg_aux {
	u32 offset;
	enum bpf_reg_type reg_type;
	u32 btf_id;
};

struct btf_mod_pair {
	struct btf *btf;
	struct module *module;
};

struct bpf_kfunc_desc_tab;

struct bpf_prog_aux {
	atomic64_t refcnt;
	u32 used_map_cnt;
	u32 used_btf_cnt;
	u32 max_ctx_offset;
	u32 max_pkt_offset;
	u32 max_tp_access;
	u32 stack_depth;
	u32 id;
	u32 func_cnt; 
	u32 func_idx; 
	u32 attach_btf_id; 
	u32 ctx_arg_info_size;
	u32 max_rdonly_access;
	u32 max_rdwr_access;
	struct btf *attach_btf;
	const struct bpf_ctx_arg_aux *ctx_arg_info;
	struct mutex dst_mutex; 
	struct bpf_prog *dst_prog;
	struct bpf_trampoline *dst_trampoline;
	enum bpf_prog_type saved_dst_prog_type;
	enum bpf_attach_type saved_dst_attach_type;
	bool verifier_zext; 
	bool offload_requested;
	bool attach_btf_trace; 
	bool func_proto_unreliable;
	bool sleepable;
	bool tail_call_reachable;
	bool xdp_has_frags;
	
	const struct btf_type *attach_func_proto;
	
	const char *attach_func_name;
	struct bpf_prog **func;
	void *jit_data; 
	struct bpf_jit_poke_descriptor *poke_tab;
	struct bpf_kfunc_desc_tab *kfunc_tab;
	struct bpf_kfunc_btf_tab *kfunc_btf_tab;
	u32 size_poke_tab;
	struct bpf_ksym ksym;
	const struct bpf_prog_ops *ops;
	struct bpf_map **used_maps;
	struct mutex used_maps_mutex; 
	struct btf_mod_pair *used_btfs;
	struct bpf_prog *prog;
	struct user_struct *user;
	u64 load_time; 
	u32 verified_insns;
	int cgroup_atype; 
	struct bpf_map *cgroup_storage[MAX_BPF_CGROUP_STORAGE_TYPE];
	char name[BPF_OBJ_NAME_LEN];
#ifdef CONFIG_SECURITY
	void *security;
#endif
	struct bpf_prog_offload *offload;
	struct btf *btf;
	struct bpf_func_info *func_info;
	struct bpf_func_info_aux *func_info_aux;
	
	struct bpf_line_info *linfo;
	
	void **jited_linfo;
	u32 func_info_cnt;
	u32 nr_linfo;
	
	u32 linfo_idx;
	u32 num_exentries;
	struct exception_table_entry *extable;
	union {
		struct work_struct work;
		struct rcu_head	rcu;
	};
};

struct bpf_prog {
	u16			pages;		
	u16			jited:1,	
				jit_requested:1,
				gpl_compatible:1, 
				cb_access:1,	
				dst_needed:1,	
				blinding_requested:1, 
				blinded:1,	
				is_func:1,	
				kprobe_override:1, 
				has_callchain_buf:1, 
				enforce_expected_attach_type:1, 
				call_get_stack:1, 
				call_get_func_ip:1, 
				tstamp_type_access:1; 
	enum bpf_prog_type	type;		
	enum bpf_attach_type	expected_attach_type; 
	u32			len;		
	u32			jited_len;	
	u8			tag[BPF_TAG_SIZE];
	struct bpf_prog_stats __percpu *stats;
	int __percpu		*active;
	unsigned int		(*bpf_func)(const void *ctx,
					    const struct bpf_insn *insn);
	struct bpf_prog_aux	*aux;		
	struct sock_fprog_kern	*orig_prog;	
	
	union {
		DECLARE_FLEX_ARRAY(struct sock_filter, insns);
		DECLARE_FLEX_ARRAY(struct bpf_insn, insnsi);
	};
};

struct bpf_array_aux {
	
	struct list_head poke_progs;
	struct bpf_map *map;
	struct mutex poke_mutex;
	struct work_struct work;
};

struct bpf_link {
	atomic64_t refcnt;
	u32 id;
	enum bpf_link_type type;
	const struct bpf_link_ops *ops;
	struct bpf_prog *prog;
	struct work_struct work;
};

struct bpf_link_ops {
	void (*release)(struct bpf_link *link);
	void (*dealloc)(struct bpf_link *link);
	int (*detach)(struct bpf_link *link);
	int (*update_prog)(struct bpf_link *link, struct bpf_prog *new_prog,
			   struct bpf_prog *old_prog);
	void (*show_fdinfo)(const struct bpf_link *link, struct seq_file *seq);
	int (*fill_link_info)(const struct bpf_link *link,
			      struct bpf_link_info *info);
};

struct bpf_tramp_link {
	struct bpf_link link;
	struct hlist_node tramp_hlist;
	u64 cookie;
};

struct bpf_shim_tramp_link {
	struct bpf_tramp_link link;
	struct bpf_trampoline *trampoline;
};

struct bpf_tracing_link {
	struct bpf_tramp_link link;
	enum bpf_attach_type attach_type;
	struct bpf_trampoline *trampoline;
	struct bpf_prog *tgt_prog;
};

struct bpf_link_primer {
	struct bpf_link *link;
	struct file *file;
	int fd;
	u32 id;
};

struct bpf_struct_ops_value;
struct btf_member;

#define BPF_STRUCT_OPS_MAX_NR_MEMBERS 64
struct bpf_struct_ops {
	const struct bpf_verifier_ops *verifier_ops;
	int (*init)(struct btf *btf);
	int (*check_member)(const struct btf_type *t,
			    const struct btf_member *member);
	int (*init_member)(const struct btf_type *t,
			   const struct btf_member *member,
			   void *kdata, const void *udata);
	int (*reg)(void *kdata);
	void (*unreg)(void *kdata);
	const struct btf_type *type;
	const struct btf_type *value_type;
	const char *name;
	struct btf_func_model func_models[BPF_STRUCT_OPS_MAX_NR_MEMBERS];
	u32 type_id;
	u32 value_id;
};

#if defined(CONFIG_BPF_JIT) && defined(CONFIG_BPF_SYSCALL)
#define BPF_MODULE_OWNER ((void *)((0xeB9FUL << 2) + POISON_POINTER_DELTA))
const struct bpf_struct_ops *bpf_struct_ops_find(u32 type_id);
void bpf_struct_ops_init(struct btf *btf, struct bpf_verifier_log *log);
bool bpf_struct_ops_get(const void *kdata);
void bpf_struct_ops_put(const void *kdata);
int bpf_struct_ops_map_sys_lookup_elem(struct bpf_map *map, void *key,
				       void *value);
int bpf_struct_ops_prepare_trampoline(struct bpf_tramp_links *tlinks,
				      struct bpf_tramp_link *link,
				      const struct btf_func_model *model,
				      void *image, void *image_end);
static inline bool bpf_try_module_get(const void *data, struct module *owner)
{
	if (owner == BPF_MODULE_OWNER)
		return bpf_struct_ops_get(data);
	else
		return try_module_get(owner);
}
static inline void bpf_module_put(const void *data, struct module *owner)
{
	if (owner == BPF_MODULE_OWNER)
		bpf_struct_ops_put(data);
	else
		module_put(owner);
}

#ifdef CONFIG_NET

struct bpf_dummy_ops_state {
	int val;
};

struct bpf_dummy_ops {
	int (*test_1)(struct bpf_dummy_ops_state *cb);
	int (*test_2)(struct bpf_dummy_ops_state *cb, int a1, unsigned short a2,
		      char a3, unsigned long a4);
};

int bpf_struct_ops_test_run(struct bpf_prog *prog, const union bpf_attr *kattr,
			    union bpf_attr __user *uattr);
#endif
#else
static inline const struct bpf_struct_ops *bpf_struct_ops_find(u32 type_id)
{
	return NULL;
}
static inline void bpf_struct_ops_init(struct btf *btf,
				       struct bpf_verifier_log *log)
{
}
static inline bool bpf_try_module_get(const void *data, struct module *owner)
{
	return try_module_get(owner);
}
static inline void bpf_module_put(const void *data, struct module *owner)
{
	module_put(owner);
}
static inline int bpf_struct_ops_map_sys_lookup_elem(struct bpf_map *map,
						     void *key,
						     void *value)
{
	return -EINVAL;
}
#endif

#if defined(CONFIG_CGROUP_BPF) && defined(CONFIG_BPF_LSM)
int bpf_trampoline_link_cgroup_shim(struct bpf_prog *prog,
				    int cgroup_atype);
void bpf_trampoline_unlink_cgroup_shim(struct bpf_prog *prog);
#else
static inline int bpf_trampoline_link_cgroup_shim(struct bpf_prog *prog,
						  int cgroup_atype)
{
	return -EOPNOTSUPP;
}
static inline void bpf_trampoline_unlink_cgroup_shim(struct bpf_prog *prog)
{
}
#endif

struct bpf_array {
	struct bpf_map map;
	u32 elem_size;
	u32 index_mask;
	struct bpf_array_aux *aux;
	union {
		char value[0] __aligned(8);
		void *ptrs[0] __aligned(8);
		void __percpu *pptrs[0] __aligned(8);
	};
};

#define BPF_COMPLEXITY_LIMIT_INSNS      1000000 
#define MAX_TAIL_CALL_CNT 33


#define BPF_MAX_LOOPS	BIT(23)

#define BPF_F_ACCESS_MASK	(BPF_F_RDONLY |		\
				 BPF_F_RDONLY_PROG |	\
				 BPF_F_WRONLY |		\
				 BPF_F_WRONLY_PROG)

#define BPF_MAP_CAN_READ	BIT(0)
#define BPF_MAP_CAN_WRITE	BIT(1)

static inline u32 bpf_map_flags_to_cap(struct bpf_map *map)
{
	u32 access_flags = map->map_flags & (BPF_F_RDONLY_PROG | BPF_F_WRONLY_PROG);

	
	if (access_flags & BPF_F_RDONLY_PROG)
		return BPF_MAP_CAN_READ;
	else if (access_flags & BPF_F_WRONLY_PROG)
		return BPF_MAP_CAN_WRITE;
	else
		return BPF_MAP_CAN_READ | BPF_MAP_CAN_WRITE;
}

static inline bool bpf_map_flags_access_ok(u32 access_flags)
{
	return (access_flags & (BPF_F_RDONLY_PROG | BPF_F_WRONLY_PROG)) !=
	       (BPF_F_RDONLY_PROG | BPF_F_WRONLY_PROG);
}

struct bpf_event_entry {
	struct perf_event *event;
	struct file *perf_file;
	struct file *map_file;
	struct rcu_head rcu;
};

static inline bool map_type_contains_progs(struct bpf_map *map)
{
	return map->map_type == BPF_MAP_TYPE_PROG_ARRAY ||
	       map->map_type == BPF_MAP_TYPE_DEVMAP ||
	       map->map_type == BPF_MAP_TYPE_CPUMAP;
}

bool bpf_prog_map_compatible(struct bpf_map *map, const struct bpf_prog *fp);
int bpf_prog_calc_tag(struct bpf_prog *fp);

const struct bpf_func_proto *bpf_get_trace_printk_proto(void);
const struct bpf_func_proto *bpf_get_trace_vprintk_proto(void);

typedef unsigned long (*bpf_ctx_copy_t)(void *dst, const void *src,
					unsigned long off, unsigned long len);
typedef u32 (*bpf_convert_ctx_access_t)(enum bpf_access_type type,
					const struct bpf_insn *src,
					struct bpf_insn *dst,
					struct bpf_prog *prog,
					u32 *target_size);

u64 bpf_event_output(struct bpf_map *map, u64 flags, void *meta, u64 meta_size,
		     void *ctx, u64 ctx_size, bpf_ctx_copy_t ctx_copy);


struct bpf_prog_array_item {
	struct bpf_prog *prog;
	union {
		struct bpf_cgroup_storage *cgroup_storage[MAX_BPF_CGROUP_STORAGE_TYPE];
		u64 bpf_cookie;
	};
};

struct bpf_prog_array {
	struct rcu_head rcu;
	struct bpf_prog_array_item items[];
};

struct bpf_empty_prog_array {
	struct bpf_prog_array hdr;
	struct bpf_prog *null_prog;
};


extern struct bpf_empty_prog_array bpf_empty_prog_array;

struct bpf_prog_array *bpf_prog_array_alloc(u32 prog_cnt, gfp_t flags);
void bpf_prog_array_free(struct bpf_prog_array *progs);

void bpf_prog_array_free_sleepable(struct bpf_prog_array *progs);
int bpf_prog_array_length(struct bpf_prog_array *progs);
bool bpf_prog_array_is_empty(struct bpf_prog_array *array);
int bpf_prog_array_copy_to_user(struct bpf_prog_array *progs,
				__u32 __user *prog_ids, u32 cnt);

void bpf_prog_array_delete_safe(struct bpf_prog_array *progs,
				struct bpf_prog *old_prog);
int bpf_prog_array_delete_safe_at(struct bpf_prog_array *array, int index);
int bpf_prog_array_update_at(struct bpf_prog_array *array, int index,
			     struct bpf_prog *prog);
int bpf_prog_array_copy_info(struct bpf_prog_array *array,
			     u32 *prog_ids, u32 request_cnt,
			     u32 *prog_cnt);
int bpf_prog_array_copy(struct bpf_prog_array *old_array,
			struct bpf_prog *exclude_prog,
			struct bpf_prog *include_prog,
			u64 bpf_cookie,
			struct bpf_prog_array **new_array);

struct bpf_run_ctx {};

struct bpf_cg_run_ctx {
	struct bpf_run_ctx run_ctx;
	const struct bpf_prog_array_item *prog_item;
	int retval;
};

struct bpf_trace_run_ctx {
	struct bpf_run_ctx run_ctx;
	u64 bpf_cookie;
};

struct bpf_tramp_run_ctx {
	struct bpf_run_ctx run_ctx;
	u64 bpf_cookie;
	struct bpf_run_ctx *saved_run_ctx;
};

static inline struct bpf_run_ctx *bpf_set_run_ctx(struct bpf_run_ctx *new_ctx)
{
	struct bpf_run_ctx *old_ctx = NULL;

#ifdef CONFIG_BPF_SYSCALL
	old_ctx = current->bpf_ctx;
	current->bpf_ctx = new_ctx;
#endif
	return old_ctx;
}

static inline void bpf_reset_run_ctx(struct bpf_run_ctx *old_ctx)
{
#ifdef CONFIG_BPF_SYSCALL
	current->bpf_ctx = old_ctx;
#endif
}


#define BPF_RET_BIND_NO_CAP_NET_BIND_SERVICE			(1 << 0)

#define BPF_RET_SET_CN						(1 << 0)

typedef u32 (*bpf_prog_run_fn)(const struct bpf_prog *prog, const void *ctx);

static __always_inline u32
bpf_prog_run_array(const struct bpf_prog_array *array,
		   const void *ctx, bpf_prog_run_fn run_prog)
{
	const struct bpf_prog_array_item *item;
	const struct bpf_prog *prog;
	struct bpf_run_ctx *old_run_ctx;
	struct bpf_trace_run_ctx run_ctx;
	u32 ret = 1;

	RCU_LOCKDEP_WARN(!rcu_read_lock_held(), "no rcu lock held");

	if (unlikely(!array))
		return ret;

	migrate_disable();
	old_run_ctx = bpf_set_run_ctx(&run_ctx.run_ctx);
	item = &array->items[0];
	while ((prog = READ_ONCE(item->prog))) {
		run_ctx.bpf_cookie = item->bpf_cookie;
		ret &= run_prog(prog, ctx);
		item++;
	}
	bpf_reset_run_ctx(old_run_ctx);
	migrate_enable();
	return ret;
}


static __always_inline u32
bpf_prog_run_array_sleepable(const struct bpf_prog_array __rcu *array_rcu,
			     const void *ctx, bpf_prog_run_fn run_prog)
{
	const struct bpf_prog_array_item *item;
	const struct bpf_prog *prog;
	const struct bpf_prog_array *array;
	struct bpf_run_ctx *old_run_ctx;
	struct bpf_trace_run_ctx run_ctx;
	u32 ret = 1;

	might_fault();

	rcu_read_lock_trace();
	migrate_disable();

	array = rcu_dereference_check(array_rcu, rcu_read_lock_trace_held());
	if (unlikely(!array))
		goto out;
	old_run_ctx = bpf_set_run_ctx(&run_ctx.run_ctx);
	item = &array->items[0];
	while ((prog = READ_ONCE(item->prog))) {
		if (!prog->aux->sleepable)
			rcu_read_lock();

		run_ctx.bpf_cookie = item->bpf_cookie;
		ret &= run_prog(prog, ctx);
		item++;

		if (!prog->aux->sleepable)
			rcu_read_unlock();
	}
	bpf_reset_run_ctx(old_run_ctx);
out:
	migrate_enable();
	rcu_read_unlock_trace();
	return ret;
}

#ifdef CONFIG_BPF_SYSCALL
DECLARE_PER_CPU(int, bpf_prog_active);
extern struct mutex bpf_stats_enabled_mutex;


static inline void bpf_disable_instrumentation(void)
{
	migrate_disable();
	this_cpu_inc(bpf_prog_active);
}

static inline void bpf_enable_instrumentation(void)
{
	this_cpu_dec(bpf_prog_active);
	migrate_enable();
}

extern const struct file_operations bpf_map_fops;
extern const struct file_operations bpf_prog_fops;
extern const struct file_operations bpf_iter_fops;

#define BPF_PROG_TYPE(_id, _name, prog_ctx_type, kern_ctx_type) \
	extern const struct bpf_prog_ops _name ## _prog_ops; \
	extern const struct bpf_verifier_ops _name ## _verifier_ops;
#define BPF_MAP_TYPE(_id, _ops) \
	extern const struct bpf_map_ops _ops;
#define BPF_LINK_TYPE(_id, _name)
#include <linux/bpf_types.h>
#undef BPF_PROG_TYPE
#undef BPF_MAP_TYPE
#undef BPF_LINK_TYPE

extern const struct bpf_prog_ops bpf_offload_prog_ops;
extern const struct bpf_verifier_ops tc_cls_act_analyzer_ops;
extern const struct bpf_verifier_ops xdp_analyzer_ops;

struct bpf_prog *bpf_prog_get(u32 ufd);
struct bpf_prog *bpf_prog_get_type_dev(u32 ufd, enum bpf_prog_type type,
				       bool attach_drv);
void bpf_prog_add(struct bpf_prog *prog, int i);
void bpf_prog_sub(struct bpf_prog *prog, int i);
void bpf_prog_inc(struct bpf_prog *prog);
struct bpf_prog * __must_check bpf_prog_inc_not_zero(struct bpf_prog *prog);
void bpf_prog_put(struct bpf_prog *prog);

void bpf_prog_free_id(struct bpf_prog *prog, bool do_idr_lock);
void bpf_map_free_id(struct bpf_map *map, bool do_idr_lock);

struct bpf_map_value_off_desc *bpf_map_kptr_off_contains(struct bpf_map *map, u32 offset);
void bpf_map_free_kptr_off_tab(struct bpf_map *map);
struct bpf_map_value_off *bpf_map_copy_kptr_off_tab(const struct bpf_map *map);
bool bpf_map_equal_kptr_off_tab(const struct bpf_map *map_a, const struct bpf_map *map_b);
void bpf_map_free_kptrs(struct bpf_map *map, void *map_value);

struct bpf_map *bpf_map_get(u32 ufd);
struct bpf_map *bpf_map_get_with_uref(u32 ufd);
struct bpf_map *__bpf_map_get(struct fd f);
void bpf_map_inc(struct bpf_map *map);
void bpf_map_inc_with_uref(struct bpf_map *map);
struct bpf_map * __must_check bpf_map_inc_not_zero(struct bpf_map *map);
void bpf_map_put_with_uref(struct bpf_map *map);
void bpf_map_put(struct bpf_map *map);
void *bpf_map_area_alloc(u64 size, int numa_node);
void *bpf_map_area_mmapable_alloc(u64 size, int numa_node);
void bpf_map_area_free(void *base);
bool bpf_map_write_active(const struct bpf_map *map);
void bpf_map_init_from_attr(struct bpf_map *map, union bpf_attr *attr);
int  generic_map_lookup_batch(struct bpf_map *map,
			      const union bpf_attr *attr,
			      union bpf_attr __user *uattr);
int  generic_map_update_batch(struct bpf_map *map,
			      const union bpf_attr *attr,
			      union bpf_attr __user *uattr);
int  generic_map_delete_batch(struct bpf_map *map,
			      const union bpf_attr *attr,
			      union bpf_attr __user *uattr);
struct bpf_map *bpf_map_get_curr_or_next(u32 *id);
struct bpf_prog *bpf_prog_get_curr_or_next(u32 *id);

#ifdef CONFIG_MEMCG_KMEM
void *bpf_map_kmalloc_node(const struct bpf_map *map, size_t size, gfp_t flags,
			   int node);
void *bpf_map_kzalloc(const struct bpf_map *map, size_t size, gfp_t flags);
void __percpu *bpf_map_alloc_percpu(const struct bpf_map *map, size_t size,
				    size_t align, gfp_t flags);
#else
static inline void *
bpf_map_kmalloc_node(const struct bpf_map *map, size_t size, gfp_t flags,
		     int node)
{
	return kmalloc_node(size, flags, node);
}

static inline void *
bpf_map_kzalloc(const struct bpf_map *map, size_t size, gfp_t flags)
{
	return kzalloc(size, flags);
}

static inline void __percpu *
bpf_map_alloc_percpu(const struct bpf_map *map, size_t size, size_t align,
		     gfp_t flags)
{
	return __alloc_percpu_gfp(size, align, flags);
}
#endif

extern int sysctl_unprivileged_bpf_disabled;

static inline bool bpf_allow_ptr_leaks(void)
{
	return perfmon_capable();
}

static inline bool bpf_allow_uninit_stack(void)
{
	return perfmon_capable();
}

static inline bool bpf_allow_ptr_to_map_access(void)
{
	return perfmon_capable();
}

static inline bool bpf_bypass_spec_v1(void)
{
	return perfmon_capable();
}

static inline bool bpf_bypass_spec_v4(void)
{
	return perfmon_capable();
}

int bpf_map_new_fd(struct bpf_map *map, int flags);
int bpf_prog_new_fd(struct bpf_prog *prog);

void bpf_link_init(struct bpf_link *link, enum bpf_link_type type,
		   const struct bpf_link_ops *ops, struct bpf_prog *prog);
int bpf_link_prime(struct bpf_link *link, struct bpf_link_primer *primer);
int bpf_link_settle(struct bpf_link_primer *primer);
void bpf_link_cleanup(struct bpf_link_primer *primer);
void bpf_link_inc(struct bpf_link *link);
void bpf_link_put(struct bpf_link *link);
int bpf_link_new_fd(struct bpf_link *link);
struct file *bpf_link_new_file(struct bpf_link *link, int *reserved_fd);
struct bpf_link *bpf_link_get_from_fd(u32 ufd);
struct bpf_link *bpf_link_get_curr_or_next(u32 *id);

int bpf_obj_pin_user(u32 ufd, const char __user *pathname);
int bpf_obj_get_user(const char __user *pathname, int flags);

#define BPF_ITER_FUNC_PREFIX "bpf_iter_"
#define DEFINE_BPF_ITER_FUNC(target, args...)			\
	extern int bpf_iter_ ## target(args);			\
	int __init bpf_iter_ ## target(args) { return 0; }

struct bpf_iter_aux_info {
	struct bpf_map *map;
};

typedef int (*bpf_iter_attach_target_t)(struct bpf_prog *prog,
					union bpf_iter_link_info *linfo,
					struct bpf_iter_aux_info *aux);
typedef void (*bpf_iter_detach_target_t)(struct bpf_iter_aux_info *aux);
typedef void (*bpf_iter_show_fdinfo_t) (const struct bpf_iter_aux_info *aux,
					struct seq_file *seq);
typedef int (*bpf_iter_fill_link_info_t)(const struct bpf_iter_aux_info *aux,
					 struct bpf_link_info *info);
typedef const struct bpf_func_proto *
(*bpf_iter_get_func_proto_t)(enum bpf_func_id func_id,
			     const struct bpf_prog *prog);

enum bpf_iter_feature {
	BPF_ITER_RESCHED	= BIT(0),
};

#define BPF_ITER_CTX_ARG_MAX 2
struct bpf_iter_reg {
	const char *target;
	bpf_iter_attach_target_t attach_target;
	bpf_iter_detach_target_t detach_target;
	bpf_iter_show_fdinfo_t show_fdinfo;
	bpf_iter_fill_link_info_t fill_link_info;
	bpf_iter_get_func_proto_t get_func_proto;
	u32 ctx_arg_info_size;
	u32 feature;
	struct bpf_ctx_arg_aux ctx_arg_info[BPF_ITER_CTX_ARG_MAX];
	const struct bpf_iter_seq_info *seq_info;
};

struct bpf_iter_meta {
	__bpf_md_ptr(struct seq_file *, seq);
	u64 session_id;
	u64 seq_num;
};

struct bpf_iter__bpf_map_elem {
	__bpf_md_ptr(struct bpf_iter_meta *, meta);
	__bpf_md_ptr(struct bpf_map *, map);
	__bpf_md_ptr(void *, key);
	__bpf_md_ptr(void *, value);
};

int bpf_iter_reg_target(const struct bpf_iter_reg *reg_info);
void bpf_iter_unreg_target(const struct bpf_iter_reg *reg_info);
bool bpf_iter_prog_supported(struct bpf_prog *prog);
const struct bpf_func_proto *
bpf_iter_get_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog);
int bpf_iter_link_attach(const union bpf_attr *attr, bpfptr_t uattr, struct bpf_prog *prog);
int bpf_iter_new_fd(struct bpf_link *link);
bool bpf_link_is_iter(struct bpf_link *link);
struct bpf_prog *bpf_iter_get_info(struct bpf_iter_meta *meta, bool in_stop);
int bpf_iter_run_prog(struct bpf_prog *prog, void *ctx);
void bpf_iter_map_show_fdinfo(const struct bpf_iter_aux_info *aux,
			      struct seq_file *seq);
int bpf_iter_map_fill_link_info(const struct bpf_iter_aux_info *aux,
				struct bpf_link_info *info);

int map_set_for_each_callback_args(struct bpf_verifier_env *env,
				   struct bpf_func_state *caller,
				   struct bpf_func_state *callee);

int bpf_percpu_hash_copy(struct bpf_map *map, void *key, void *value);
int bpf_percpu_array_copy(struct bpf_map *map, void *key, void *value);
int bpf_percpu_hash_update(struct bpf_map *map, void *key, void *value,
			   u64 flags);
int bpf_percpu_array_update(struct bpf_map *map, void *key, void *value,
			    u64 flags);

int bpf_stackmap_copy(struct bpf_map *map, void *key, void *value);

int bpf_fd_array_map_update_elem(struct bpf_map *map, struct file *map_file,
				 void *key, void *value, u64 map_flags);
int bpf_fd_array_map_lookup_elem(struct bpf_map *map, void *key, u32 *value);
int bpf_fd_htab_map_update_elem(struct bpf_map *map, struct file *map_file,
				void *key, void *value, u64 map_flags);
int bpf_fd_htab_map_lookup_elem(struct bpf_map *map, void *key, u32 *value);

int bpf_get_file_flag(int flags);
int bpf_check_uarg_tail_zero(bpfptr_t uaddr, size_t expected_size,
			     size_t actual_size);


static inline void bpf_long_memcpy(void *dst, const void *src, u32 size)
{
	const long *lsrc = src;
	long *ldst = dst;

	size /= sizeof(long);
	while (size--)
		*ldst++ = *lsrc++;
}


int bpf_check(struct bpf_prog **fp, union bpf_attr *attr, bpfptr_t uattr);

#ifndef CONFIG_BPF_JIT_ALWAYS_ON
void bpf_patch_call_args(struct bpf_insn *insn, u32 stack_depth);
#endif

struct btf *bpf_get_btf_vmlinux(void);


struct xdp_frame;
struct sk_buff;
struct bpf_dtab_netdev;
struct bpf_cpu_map_entry;

void __dev_flush(void);
int dev_xdp_enqueue(struct net_device *dev, struct xdp_frame *xdpf,
		    struct net_device *dev_rx);
int dev_map_enqueue(struct bpf_dtab_netdev *dst, struct xdp_frame *xdpf,
		    struct net_device *dev_rx);
int dev_map_enqueue_multi(struct xdp_frame *xdpf, struct net_device *dev_rx,
			  struct bpf_map *map, bool exclude_ingress);
int dev_map_generic_redirect(struct bpf_dtab_netdev *dst, struct sk_buff *skb,
			     struct bpf_prog *xdp_prog);
int dev_map_redirect_multi(struct net_device *dev, struct sk_buff *skb,
			   struct bpf_prog *xdp_prog, struct bpf_map *map,
			   bool exclude_ingress);

void __cpu_map_flush(void);
int cpu_map_enqueue(struct bpf_cpu_map_entry *rcpu, struct xdp_frame *xdpf,
		    struct net_device *dev_rx);
int cpu_map_generic_redirect(struct bpf_cpu_map_entry *rcpu,
			     struct sk_buff *skb);


static inline int bpf_map_attr_numa_node(const union bpf_attr *attr)
{
	return (attr->map_flags & BPF_F_NUMA_NODE) ?
		attr->numa_node : NUMA_NO_NODE;
}

struct bpf_prog *bpf_prog_get_type_path(const char *name, enum bpf_prog_type type);
int array_map_alloc_check(union bpf_attr *attr);

int bpf_prog_test_run_xdp(struct bpf_prog *prog, const union bpf_attr *kattr,
			  union bpf_attr __user *uattr);
int bpf_prog_test_run_skb(struct bpf_prog *prog, const union bpf_attr *kattr,
			  union bpf_attr __user *uattr);
int bpf_prog_test_run_tracing(struct bpf_prog *prog,
			      const union bpf_attr *kattr,
			      union bpf_attr __user *uattr);
int bpf_prog_test_run_flow_dissector(struct bpf_prog *prog,
				     const union bpf_attr *kattr,
				     union bpf_attr __user *uattr);
int bpf_prog_test_run_raw_tp(struct bpf_prog *prog,
			     const union bpf_attr *kattr,
			     union bpf_attr __user *uattr);
int bpf_prog_test_run_sk_lookup(struct bpf_prog *prog,
				const union bpf_attr *kattr,
				union bpf_attr __user *uattr);
bool btf_ctx_access(int off, int size, enum bpf_access_type type,
		    const struct bpf_prog *prog,
		    struct bpf_insn_access_aux *info);

static inline bool bpf_tracing_ctx_access(int off, int size,
					  enum bpf_access_type type)
{
	if (off < 0 || off >= sizeof(__u64) * MAX_BPF_FUNC_ARGS)
		return false;
	if (type != BPF_READ)
		return false;
	if (off % size != 0)
		return false;
	return true;
}

static inline bool bpf_tracing_btf_ctx_access(int off, int size,
					      enum bpf_access_type type,
					      const struct bpf_prog *prog,
					      struct bpf_insn_access_aux *info)
{
	if (!bpf_tracing_ctx_access(off, size, type))
		return false;
	return btf_ctx_access(off, size, type, prog, info);
}

int btf_struct_access(struct bpf_verifier_log *log, const struct btf *btf,
		      const struct btf_type *t, int off, int size,
		      enum bpf_access_type atype,
		      u32 *next_btf_id, enum bpf_type_flag *flag);
bool btf_struct_ids_match(struct bpf_verifier_log *log,
			  const struct btf *btf, u32 id, int off,
			  const struct btf *need_btf, u32 need_type_id,
			  bool strict);

int btf_distill_func_proto(struct bpf_verifier_log *log,
			   struct btf *btf,
			   const struct btf_type *func_proto,
			   const char *func_name,
			   struct btf_func_model *m);

struct bpf_reg_state;
int btf_check_subprog_arg_match(struct bpf_verifier_env *env, int subprog,
				struct bpf_reg_state *regs);
int btf_check_kfunc_arg_match(struct bpf_verifier_env *env,
			      const struct btf *btf, u32 func_id,
			      struct bpf_reg_state *regs,
			      u32 kfunc_flags);
int btf_prepare_func_args(struct bpf_verifier_env *env, int subprog,
			  struct bpf_reg_state *reg);
int btf_check_type_match(struct bpf_verifier_log *log, const struct bpf_prog *prog,
			 struct btf *btf, const struct btf_type *t);

struct bpf_prog *bpf_prog_by_id(u32 id);
struct bpf_link *bpf_link_by_id(u32 id);

const struct bpf_func_proto *bpf_base_func_proto(enum bpf_func_id func_id);
void bpf_task_storage_free(struct task_struct *task);
bool bpf_prog_has_kfunc_call(const struct bpf_prog *prog);
const struct btf_func_model *
bpf_jit_find_kfunc_model(const struct bpf_prog *prog,
			 const struct bpf_insn *insn);
struct bpf_core_ctx {
	struct bpf_verifier_log *log;
	const struct btf *btf;
};

int bpf_core_apply(struct bpf_core_ctx *ctx, const struct bpf_core_relo *relo,
		   int relo_idx, void *insn);

static inline bool unprivileged_ebpf_enabled(void)
{
	return !sysctl_unprivileged_bpf_disabled;
}

void notrace bpf_prog_inc_misses_counter(struct bpf_prog *prog);
#else 
static inline struct bpf_prog *bpf_prog_get(u32 ufd)
{
	return ERR_PTR(-EOPNOTSUPP);
}

static inline struct bpf_prog *bpf_prog_get_type_dev(u32 ufd,
						     enum bpf_prog_type type,
						     bool attach_drv)
{
	return ERR_PTR(-EOPNOTSUPP);
}

static inline void bpf_prog_add(struct bpf_prog *prog, int i)
{
}

static inline void bpf_prog_sub(struct bpf_prog *prog, int i)
{
}

static inline void bpf_prog_put(struct bpf_prog *prog)
{
}

static inline void bpf_prog_inc(struct bpf_prog *prog)
{
}

static inline struct bpf_prog *__must_check
bpf_prog_inc_not_zero(struct bpf_prog *prog)
{
	return ERR_PTR(-EOPNOTSUPP);
}

static inline void bpf_link_init(struct bpf_link *link, enum bpf_link_type type,
				 const struct bpf_link_ops *ops,
				 struct bpf_prog *prog)
{
}

static inline int bpf_link_prime(struct bpf_link *link,
				 struct bpf_link_primer *primer)
{
	return -EOPNOTSUPP;
}

static inline int bpf_link_settle(struct bpf_link_primer *primer)
{
	return -EOPNOTSUPP;
}

static inline void bpf_link_cleanup(struct bpf_link_primer *primer)
{
}

static inline void bpf_link_inc(struct bpf_link *link)
{
}

static inline void bpf_link_put(struct bpf_link *link)
{
}

static inline int bpf_obj_get_user(const char __user *pathname, int flags)
{
	return -EOPNOTSUPP;
}

static inline void __dev_flush(void)
{
}

struct xdp_frame;
struct bpf_dtab_netdev;
struct bpf_cpu_map_entry;

static inline
int dev_xdp_enqueue(struct net_device *dev, struct xdp_frame *xdpf,
		    struct net_device *dev_rx)
{
	return 0;
}

static inline
int dev_map_enqueue(struct bpf_dtab_netdev *dst, struct xdp_frame *xdpf,
		    struct net_device *dev_rx)
{
	return 0;
}

static inline
int dev_map_enqueue_multi(struct xdp_frame *xdpf, struct net_device *dev_rx,
			  struct bpf_map *map, bool exclude_ingress)
{
	return 0;
}

struct sk_buff;

static inline int dev_map_generic_redirect(struct bpf_dtab_netdev *dst,
					   struct sk_buff *skb,
					   struct bpf_prog *xdp_prog)
{
	return 0;
}

static inline
int dev_map_redirect_multi(struct net_device *dev, struct sk_buff *skb,
			   struct bpf_prog *xdp_prog, struct bpf_map *map,
			   bool exclude_ingress)
{
	return 0;
}

static inline void __cpu_map_flush(void)
{
}

static inline int cpu_map_enqueue(struct bpf_cpu_map_entry *rcpu,
				  struct xdp_frame *xdpf,
				  struct net_device *dev_rx)
{
	return 0;
}

static inline int cpu_map_generic_redirect(struct bpf_cpu_map_entry *rcpu,
					   struct sk_buff *skb)
{
	return -EOPNOTSUPP;
}

static inline struct bpf_prog *bpf_prog_get_type_path(const char *name,
				enum bpf_prog_type type)
{
	return ERR_PTR(-EOPNOTSUPP);
}

static inline int bpf_prog_test_run_xdp(struct bpf_prog *prog,
					const union bpf_attr *kattr,
					union bpf_attr __user *uattr)
{
	return -ENOTSUPP;
}

static inline int bpf_prog_test_run_skb(struct bpf_prog *prog,
					const union bpf_attr *kattr,
					union bpf_attr __user *uattr)
{
	return -ENOTSUPP;
}

static inline int bpf_prog_test_run_tracing(struct bpf_prog *prog,
					    const union bpf_attr *kattr,
					    union bpf_attr __user *uattr)
{
	return -ENOTSUPP;
}

static inline int bpf_prog_test_run_flow_dissector(struct bpf_prog *prog,
						   const union bpf_attr *kattr,
						   union bpf_attr __user *uattr)
{
	return -ENOTSUPP;
}

static inline int bpf_prog_test_run_sk_lookup(struct bpf_prog *prog,
					      const union bpf_attr *kattr,
					      union bpf_attr __user *uattr)
{
	return -ENOTSUPP;
}

static inline void bpf_map_put(struct bpf_map *map)
{
}

static inline struct bpf_prog *bpf_prog_by_id(u32 id)
{
	return ERR_PTR(-ENOTSUPP);
}

static inline const struct bpf_func_proto *
bpf_base_func_proto(enum bpf_func_id func_id)
{
	return NULL;
}

static inline void bpf_task_storage_free(struct task_struct *task)
{
}

static inline bool bpf_prog_has_kfunc_call(const struct bpf_prog *prog)
{
	return false;
}

static inline const struct btf_func_model *
bpf_jit_find_kfunc_model(const struct bpf_prog *prog,
			 const struct bpf_insn *insn)
{
	return NULL;
}

static inline bool unprivileged_ebpf_enabled(void)
{
	return false;
}

static inline void bpf_prog_inc_misses_counter(struct bpf_prog *prog)
{
}
#endif 

void __bpf_free_used_btfs(struct bpf_prog_aux *aux,
			  struct btf_mod_pair *used_btfs, u32 len);

static inline struct bpf_prog *bpf_prog_get_type(u32 ufd,
						 enum bpf_prog_type type)
{
	return bpf_prog_get_type_dev(ufd, type, false);
}

void __bpf_free_used_maps(struct bpf_prog_aux *aux,
			  struct bpf_map **used_maps, u32 len);

bool bpf_prog_get_ok(struct bpf_prog *, enum bpf_prog_type *, bool);

int bpf_prog_offload_compile(struct bpf_prog *prog);
void bpf_prog_offload_destroy(struct bpf_prog *prog);
int bpf_prog_offload_info_fill(struct bpf_prog_info *info,
			       struct bpf_prog *prog);

int bpf_map_offload_info_fill(struct bpf_map_info *info, struct bpf_map *map);

int bpf_map_offload_lookup_elem(struct bpf_map *map, void *key, void *value);
int bpf_map_offload_update_elem(struct bpf_map *map,
				void *key, void *value, u64 flags);
int bpf_map_offload_delete_elem(struct bpf_map *map, void *key);
int bpf_map_offload_get_next_key(struct bpf_map *map,
				 void *key, void *next_key);

bool bpf_offload_prog_map_match(struct bpf_prog *prog, struct bpf_map *map);

struct bpf_offload_dev *
bpf_offload_dev_create(const struct bpf_prog_offload_ops *ops, void *priv);
void bpf_offload_dev_destroy(struct bpf_offload_dev *offdev);
void *bpf_offload_dev_priv(struct bpf_offload_dev *offdev);
int bpf_offload_dev_netdev_register(struct bpf_offload_dev *offdev,
				    struct net_device *netdev);
void bpf_offload_dev_netdev_unregister(struct bpf_offload_dev *offdev,
				       struct net_device *netdev);
bool bpf_offload_dev_match(struct bpf_prog *prog, struct net_device *netdev);

void unpriv_ebpf_notify(int new_state);

#if defined(CONFIG_NET) && defined(CONFIG_BPF_SYSCALL)
int bpf_prog_offload_init(struct bpf_prog *prog, union bpf_attr *attr);

static inline bool bpf_prog_is_dev_bound(const struct bpf_prog_aux *aux)
{
	return aux->offload_requested;
}

static inline bool bpf_map_is_dev_bound(struct bpf_map *map)
{
	return unlikely(map->ops == &bpf_map_offload_ops);
}

struct bpf_map *bpf_map_offload_map_alloc(union bpf_attr *attr);
void bpf_map_offload_map_free(struct bpf_map *map);
int bpf_prog_test_run_syscall(struct bpf_prog *prog,
			      const union bpf_attr *kattr,
			      union bpf_attr __user *uattr);

int sock_map_get_from_fd(const union bpf_attr *attr, struct bpf_prog *prog);
int sock_map_prog_detach(const union bpf_attr *attr, enum bpf_prog_type ptype);
int sock_map_update_elem_sys(struct bpf_map *map, void *key, void *value, u64 flags);
int sock_map_bpf_prog_query(const union bpf_attr *attr,
			    union bpf_attr __user *uattr);

void sock_map_unhash(struct sock *sk);
void sock_map_destroy(struct sock *sk);
void sock_map_close(struct sock *sk, long timeout);
#else
static inline int bpf_prog_offload_init(struct bpf_prog *prog,
					union bpf_attr *attr)
{
	return -EOPNOTSUPP;
}

static inline bool bpf_prog_is_dev_bound(struct bpf_prog_aux *aux)
{
	return false;
}

static inline bool bpf_map_is_dev_bound(struct bpf_map *map)
{
	return false;
}

static inline struct bpf_map *bpf_map_offload_map_alloc(union bpf_attr *attr)
{
	return ERR_PTR(-EOPNOTSUPP);
}

static inline void bpf_map_offload_map_free(struct bpf_map *map)
{
}

static inline int bpf_prog_test_run_syscall(struct bpf_prog *prog,
					    const union bpf_attr *kattr,
					    union bpf_attr __user *uattr)
{
	return -ENOTSUPP;
}

#ifdef CONFIG_BPF_SYSCALL
static inline int sock_map_get_from_fd(const union bpf_attr *attr,
				       struct bpf_prog *prog)
{
	return -EINVAL;
}

static inline int sock_map_prog_detach(const union bpf_attr *attr,
				       enum bpf_prog_type ptype)
{
	return -EOPNOTSUPP;
}

static inline int sock_map_update_elem_sys(struct bpf_map *map, void *key, void *value,
					   u64 flags)
{
	return -EOPNOTSUPP;
}

static inline int sock_map_bpf_prog_query(const union bpf_attr *attr,
					  union bpf_attr __user *uattr)
{
	return -EINVAL;
}
#endif 
#endif 

#if defined(CONFIG_INET) && defined(CONFIG_BPF_SYSCALL)
void bpf_sk_reuseport_detach(struct sock *sk);
int bpf_fd_reuseport_array_lookup_elem(struct bpf_map *map, void *key,
				       void *value);
int bpf_fd_reuseport_array_update_elem(struct bpf_map *map, void *key,
				       void *value, u64 map_flags);
#else
static inline void bpf_sk_reuseport_detach(struct sock *sk)
{
}

#ifdef CONFIG_BPF_SYSCALL
static inline int bpf_fd_reuseport_array_lookup_elem(struct bpf_map *map,
						     void *key, void *value)
{
	return -EOPNOTSUPP;
}

static inline int bpf_fd_reuseport_array_update_elem(struct bpf_map *map,
						     void *key, void *value,
						     u64 map_flags)
{
	return -EOPNOTSUPP;
}
#endif 
#endif 


extern const struct bpf_func_proto bpf_map_lookup_elem_proto;
extern const struct bpf_func_proto bpf_map_update_elem_proto;
extern const struct bpf_func_proto bpf_map_delete_elem_proto;
extern const struct bpf_func_proto bpf_map_push_elem_proto;
extern const struct bpf_func_proto bpf_map_pop_elem_proto;
extern const struct bpf_func_proto bpf_map_peek_elem_proto;
extern const struct bpf_func_proto bpf_map_lookup_percpu_elem_proto;

extern const struct bpf_func_proto bpf_get_prandom_u32_proto;
extern const struct bpf_func_proto bpf_get_smp_processor_id_proto;
extern const struct bpf_func_proto bpf_get_numa_node_id_proto;
extern const struct bpf_func_proto bpf_tail_call_proto;
extern const struct bpf_func_proto bpf_ktime_get_ns_proto;
extern const struct bpf_func_proto bpf_ktime_get_boot_ns_proto;
extern const struct bpf_func_proto bpf_get_current_pid_tgid_proto;
extern const struct bpf_func_proto bpf_get_current_uid_gid_proto;
extern const struct bpf_func_proto bpf_get_current_comm_proto;
extern const struct bpf_func_proto bpf_get_stackid_proto;
extern const struct bpf_func_proto bpf_get_stack_proto;
extern const struct bpf_func_proto bpf_get_task_stack_proto;
extern const struct bpf_func_proto bpf_get_stackid_proto_pe;
extern const struct bpf_func_proto bpf_get_stack_proto_pe;
extern const struct bpf_func_proto bpf_sock_map_update_proto;
extern const struct bpf_func_proto bpf_sock_hash_update_proto;
extern const struct bpf_func_proto bpf_get_current_cgroup_id_proto;
extern const struct bpf_func_proto bpf_get_current_ancestor_cgroup_id_proto;
extern const struct bpf_func_proto bpf_msg_redirect_hash_proto;
extern const struct bpf_func_proto bpf_msg_redirect_map_proto;
extern const struct bpf_func_proto bpf_sk_redirect_hash_proto;
extern const struct bpf_func_proto bpf_sk_redirect_map_proto;
extern const struct bpf_func_proto bpf_spin_lock_proto;
extern const struct bpf_func_proto bpf_spin_unlock_proto;
extern const struct bpf_func_proto bpf_get_local_storage_proto;
extern const struct bpf_func_proto bpf_strtol_proto;
extern const struct bpf_func_proto bpf_strtoul_proto;
extern const struct bpf_func_proto bpf_tcp_sock_proto;
extern const struct bpf_func_proto bpf_jiffies64_proto;
extern const struct bpf_func_proto bpf_get_ns_current_pid_tgid_proto;
extern const struct bpf_func_proto bpf_event_output_data_proto;
extern const struct bpf_func_proto bpf_ringbuf_output_proto;
extern const struct bpf_func_proto bpf_ringbuf_reserve_proto;
extern const struct bpf_func_proto bpf_ringbuf_submit_proto;
extern const struct bpf_func_proto bpf_ringbuf_discard_proto;
extern const struct bpf_func_proto bpf_ringbuf_query_proto;
extern const struct bpf_func_proto bpf_ringbuf_reserve_dynptr_proto;
extern const struct bpf_func_proto bpf_ringbuf_submit_dynptr_proto;
extern const struct bpf_func_proto bpf_ringbuf_discard_dynptr_proto;
extern const struct bpf_func_proto bpf_skc_to_tcp6_sock_proto;
extern const struct bpf_func_proto bpf_skc_to_tcp_sock_proto;
extern const struct bpf_func_proto bpf_skc_to_tcp_timewait_sock_proto;
extern const struct bpf_func_proto bpf_skc_to_tcp_request_sock_proto;
extern const struct bpf_func_proto bpf_skc_to_udp6_sock_proto;
extern const struct bpf_func_proto bpf_skc_to_unix_sock_proto;
extern const struct bpf_func_proto bpf_skc_to_mptcp_sock_proto;
extern const struct bpf_func_proto bpf_copy_from_user_proto;
extern const struct bpf_func_proto bpf_snprintf_btf_proto;
extern const struct bpf_func_proto bpf_snprintf_proto;
extern const struct bpf_func_proto bpf_per_cpu_ptr_proto;
extern const struct bpf_func_proto bpf_this_cpu_ptr_proto;
extern const struct bpf_func_proto bpf_ktime_get_coarse_ns_proto;
extern const struct bpf_func_proto bpf_sock_from_file_proto;
extern const struct bpf_func_proto bpf_get_socket_ptr_cookie_proto;
extern const struct bpf_func_proto bpf_task_storage_get_proto;
extern const struct bpf_func_proto bpf_task_storage_delete_proto;
extern const struct bpf_func_proto bpf_for_each_map_elem_proto;
extern const struct bpf_func_proto bpf_btf_find_by_name_kind_proto;
extern const struct bpf_func_proto bpf_sk_setsockopt_proto;
extern const struct bpf_func_proto bpf_sk_getsockopt_proto;
extern const struct bpf_func_proto bpf_unlocked_sk_setsockopt_proto;
extern const struct bpf_func_proto bpf_unlocked_sk_getsockopt_proto;
extern const struct bpf_func_proto bpf_find_vma_proto;
extern const struct bpf_func_proto bpf_loop_proto;
extern const struct bpf_func_proto bpf_copy_from_user_task_proto;
extern const struct bpf_func_proto bpf_set_retval_proto;
extern const struct bpf_func_proto bpf_get_retval_proto;

const struct bpf_func_proto *tracing_prog_func_proto(
  enum bpf_func_id func_id, const struct bpf_prog *prog);


void bpf_user_rnd_init_once(void);
u64 bpf_user_rnd_u32(u64 r1, u64 r2, u64 r3, u64 r4, u64 r5);
u64 bpf_get_raw_cpu_id(u64 r1, u64 r2, u64 r3, u64 r4, u64 r5);

#if defined(CONFIG_NET)
bool bpf_sock_common_is_valid_access(int off, int size,
				     enum bpf_access_type type,
				     struct bpf_insn_access_aux *info);
bool bpf_sock_is_valid_access(int off, int size, enum bpf_access_type type,
			      struct bpf_insn_access_aux *info);
u32 bpf_sock_convert_ctx_access(enum bpf_access_type type,
				const struct bpf_insn *si,
				struct bpf_insn *insn_buf,
				struct bpf_prog *prog,
				u32 *target_size);
#else
static inline bool bpf_sock_common_is_valid_access(int off, int size,
						   enum bpf_access_type type,
						   struct bpf_insn_access_aux *info)
{
	return false;
}
static inline bool bpf_sock_is_valid_access(int off, int size,
					    enum bpf_access_type type,
					    struct bpf_insn_access_aux *info)
{
	return false;
}
static inline u32 bpf_sock_convert_ctx_access(enum bpf_access_type type,
					      const struct bpf_insn *si,
					      struct bpf_insn *insn_buf,
					      struct bpf_prog *prog,
					      u32 *target_size)
{
	return 0;
}
#endif

#ifdef CONFIG_INET
struct sk_reuseport_kern {
	struct sk_buff *skb;
	struct sock *sk;
	struct sock *selected_sk;
	struct sock *migrating_sk;
	void *data_end;
	u32 hash;
	u32 reuseport_id;
	bool bind_inany;
};
bool bpf_tcp_sock_is_valid_access(int off, int size, enum bpf_access_type type,
				  struct bpf_insn_access_aux *info);

u32 bpf_tcp_sock_convert_ctx_access(enum bpf_access_type type,
				    const struct bpf_insn *si,
				    struct bpf_insn *insn_buf,
				    struct bpf_prog *prog,
				    u32 *target_size);

bool bpf_xdp_sock_is_valid_access(int off, int size, enum bpf_access_type type,
				  struct bpf_insn_access_aux *info);

u32 bpf_xdp_sock_convert_ctx_access(enum bpf_access_type type,
				    const struct bpf_insn *si,
				    struct bpf_insn *insn_buf,
				    struct bpf_prog *prog,
				    u32 *target_size);
#else
static inline bool bpf_tcp_sock_is_valid_access(int off, int size,
						enum bpf_access_type type,
						struct bpf_insn_access_aux *info)
{
	return false;
}

static inline u32 bpf_tcp_sock_convert_ctx_access(enum bpf_access_type type,
						  const struct bpf_insn *si,
						  struct bpf_insn *insn_buf,
						  struct bpf_prog *prog,
						  u32 *target_size)
{
	return 0;
}
static inline bool bpf_xdp_sock_is_valid_access(int off, int size,
						enum bpf_access_type type,
						struct bpf_insn_access_aux *info)
{
	return false;
}

static inline u32 bpf_xdp_sock_convert_ctx_access(enum bpf_access_type type,
						  const struct bpf_insn *si,
						  struct bpf_insn *insn_buf,
						  struct bpf_prog *prog,
						  u32 *target_size)
{
	return 0;
}
#endif 

enum bpf_text_poke_type {
	BPF_MOD_CALL,
	BPF_MOD_JUMP,
};

int bpf_arch_text_poke(void *ip, enum bpf_text_poke_type t,
		       void *addr1, void *addr2);

void *bpf_arch_text_copy(void *dst, void *src, size_t len);
int bpf_arch_text_invalidate(void *dst, size_t len);

struct btf_id_set;
bool btf_id_set_contains(const struct btf_id_set *set, u32 id);

#define MAX_BPRINTF_VARARGS		12

int bpf_bprintf_prepare(char *fmt, u32 fmt_size, const u64 *raw_args,
			u32 **bin_buf, u32 num_args);
void bpf_bprintf_cleanup(void);


struct bpf_dynptr_kern {
	void *data;
	
	u32 size;
	u32 offset;
} __aligned(8);

enum bpf_dynptr_type {
	BPF_DYNPTR_TYPE_INVALID,
	
	BPF_DYNPTR_TYPE_LOCAL,
	
	BPF_DYNPTR_TYPE_RINGBUF,
};

void bpf_dynptr_init(struct bpf_dynptr_kern *ptr, void *data,
		     enum bpf_dynptr_type type, u32 offset, u32 size);
void bpf_dynptr_set_null(struct bpf_dynptr_kern *ptr);
int bpf_dynptr_check_size(u32 size);

#ifdef CONFIG_BPF_LSM
void bpf_cgroup_atype_get(u32 attach_btf_id, int cgroup_atype);
void bpf_cgroup_atype_put(int cgroup_atype);
#else
static inline void bpf_cgroup_atype_get(u32 attach_btf_id, int cgroup_atype) {}
static inline void bpf_cgroup_atype_put(int cgroup_atype) {}
#endif 

#endif 
