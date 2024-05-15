/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MNT_IDMAPPING_H
#define _LINUX_MNT_IDMAPPING_H

#include <linux/types.h>
#include <linux/uidgid.h>

struct user_namespace;

extern struct user_namespace init_user_ns;

typedef struct {
	uid_t val;
} vfsuid_t;

typedef struct {
	gid_t val;
} vfsgid_t;

static_assert(sizeof(vfsuid_t) == sizeof(kuid_t));
static_assert(sizeof(vfsgid_t) == sizeof(kgid_t));
static_assert(offsetof(vfsuid_t, val) == offsetof(kuid_t, val));
static_assert(offsetof(vfsgid_t, val) == offsetof(kgid_t, val));

#ifdef CONFIG_MULTIUSER
static inline uid_t __vfsuid_val(vfsuid_t uid)
{
	return uid.val;
}

static inline gid_t __vfsgid_val(vfsgid_t gid)
{
	return gid.val;
}
#else
static inline uid_t __vfsuid_val(vfsuid_t uid)
{
	return 0;
}

static inline gid_t __vfsgid_val(vfsgid_t gid)
{
	return 0;
}
#endif

static inline bool vfsuid_valid(vfsuid_t uid)
{
	return __vfsuid_val(uid) != (uid_t)-1;
}

static inline bool vfsgid_valid(vfsgid_t gid)
{
	return __vfsgid_val(gid) != (gid_t)-1;
}

static inline bool vfsuid_eq(vfsuid_t left, vfsuid_t right)
{
	return vfsuid_valid(left) && __vfsuid_val(left) == __vfsuid_val(right);
}

static inline bool vfsgid_eq(vfsgid_t left, vfsgid_t right)
{
	return vfsgid_valid(left) && __vfsgid_val(left) == __vfsgid_val(right);
}


static inline bool vfsuid_eq_kuid(vfsuid_t vfsuid, kuid_t kuid)
{
	return vfsuid_valid(vfsuid) && __vfsuid_val(vfsuid) == __kuid_val(kuid);
}


static inline bool vfsgid_eq_kgid(vfsgid_t vfsgid, kgid_t kgid)
{
	return vfsgid_valid(vfsgid) && __vfsgid_val(vfsgid) == __kgid_val(kgid);
}


#define VFSUIDT_INIT(val) (vfsuid_t){ __kuid_val(val) }
#define VFSGIDT_INIT(val) (vfsgid_t){ __kgid_val(val) }

#define INVALID_VFSUID VFSUIDT_INIT(INVALID_UID)
#define INVALID_VFSGID VFSGIDT_INIT(INVALID_GID)


#define AS_KUIDT(val) (kuid_t){ __vfsuid_val(val) }
#define AS_KGIDT(val) (kgid_t){ __vfsgid_val(val) }

#ifdef CONFIG_MULTIUSER

static inline int vfsgid_in_group_p(vfsgid_t vfsgid)
{
	return in_group_p(AS_KGIDT(vfsgid));
}
#else
static inline int vfsgid_in_group_p(vfsgid_t vfsgid)
{
	return 1;
}
#endif


static inline bool initial_idmapping(const struct user_namespace *ns)
{
	return ns == &init_user_ns;
}


static inline bool no_idmapping(const struct user_namespace *mnt_userns,
				const struct user_namespace *fs_userns)
{
	return initial_idmapping(mnt_userns) || mnt_userns == fs_userns;
}



static inline vfsuid_t make_vfsuid(struct user_namespace *mnt_userns,
				   struct user_namespace *fs_userns,
				   kuid_t kuid)
{
	uid_t uid;

	if (no_idmapping(mnt_userns, fs_userns))
		return VFSUIDT_INIT(kuid);
	if (initial_idmapping(fs_userns))
		uid = __kuid_val(kuid);
	else
		uid = from_kuid(fs_userns, kuid);
	if (uid == (uid_t)-1)
		return INVALID_VFSUID;
	return VFSUIDT_INIT(make_kuid(mnt_userns, uid));
}

static inline kuid_t mapped_kuid_fs(struct user_namespace *mnt_userns,
				    struct user_namespace *fs_userns,
				    kuid_t kuid)
{
	return AS_KUIDT(make_vfsuid(mnt_userns, fs_userns, kuid));
}



static inline vfsgid_t make_vfsgid(struct user_namespace *mnt_userns,
				   struct user_namespace *fs_userns,
				   kgid_t kgid)
{
	gid_t gid;

	if (no_idmapping(mnt_userns, fs_userns))
		return VFSGIDT_INIT(kgid);
	if (initial_idmapping(fs_userns))
		gid = __kgid_val(kgid);
	else
		gid = from_kgid(fs_userns, kgid);
	if (gid == (gid_t)-1)
		return INVALID_VFSGID;
	return VFSGIDT_INIT(make_kgid(mnt_userns, gid));
}

static inline kgid_t mapped_kgid_fs(struct user_namespace *mnt_userns,
				    struct user_namespace *fs_userns,
				    kgid_t kgid)
{
	return AS_KGIDT(make_vfsgid(mnt_userns, fs_userns, kgid));
}


static inline kuid_t from_vfsuid(struct user_namespace *mnt_userns,
				 struct user_namespace *fs_userns,
				 vfsuid_t vfsuid)
{
	uid_t uid;

	if (no_idmapping(mnt_userns, fs_userns))
		return AS_KUIDT(vfsuid);
	uid = from_kuid(mnt_userns, AS_KUIDT(vfsuid));
	if (uid == (uid_t)-1)
		return INVALID_UID;
	if (initial_idmapping(fs_userns))
		return KUIDT_INIT(uid);
	return make_kuid(fs_userns, uid);
}


static inline kuid_t mapped_kuid_user(struct user_namespace *mnt_userns,
				      struct user_namespace *fs_userns,
				      kuid_t kuid)
{
	return from_vfsuid(mnt_userns, fs_userns, VFSUIDT_INIT(kuid));
}


static inline bool vfsuid_has_fsmapping(struct user_namespace *mnt_userns,
					struct user_namespace *fs_userns,
					vfsuid_t vfsuid)
{
	return uid_valid(from_vfsuid(mnt_userns, fs_userns, vfsuid));
}


static inline kuid_t vfsuid_into_kuid(vfsuid_t vfsuid)
{
	return AS_KUIDT(vfsuid);
}


static inline kgid_t from_vfsgid(struct user_namespace *mnt_userns,
				 struct user_namespace *fs_userns,
				 vfsgid_t vfsgid)
{
	gid_t gid;

	if (no_idmapping(mnt_userns, fs_userns))
		return AS_KGIDT(vfsgid);
	gid = from_kgid(mnt_userns, AS_KGIDT(vfsgid));
	if (gid == (gid_t)-1)
		return INVALID_GID;
	if (initial_idmapping(fs_userns))
		return KGIDT_INIT(gid);
	return make_kgid(fs_userns, gid);
}


static inline kgid_t mapped_kgid_user(struct user_namespace *mnt_userns,
				      struct user_namespace *fs_userns,
				      kgid_t kgid)
{
	return from_vfsgid(mnt_userns, fs_userns, VFSGIDT_INIT(kgid));
}


static inline bool vfsgid_has_fsmapping(struct user_namespace *mnt_userns,
					struct user_namespace *fs_userns,
					vfsgid_t vfsgid)
{
	return gid_valid(from_vfsgid(mnt_userns, fs_userns, vfsgid));
}


static inline kgid_t vfsgid_into_kgid(vfsgid_t vfsgid)
{
	return AS_KGIDT(vfsgid);
}


static inline kuid_t mapped_fsuid(struct user_namespace *mnt_userns,
				  struct user_namespace *fs_userns)
{
	return from_vfsuid(mnt_userns, fs_userns,
			   VFSUIDT_INIT(current_fsuid()));
}


static inline kgid_t mapped_fsgid(struct user_namespace *mnt_userns,
				  struct user_namespace *fs_userns)
{
	return from_vfsgid(mnt_userns, fs_userns,
			   VFSGIDT_INIT(current_fsgid()));
}

#endif 
