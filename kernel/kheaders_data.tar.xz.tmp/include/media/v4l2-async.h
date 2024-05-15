/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef V4L2_ASYNC_H
#define V4L2_ASYNC_H

#include <linux/list.h>
#include <linux/mutex.h>

struct dentry;
struct device;
struct device_node;
struct v4l2_device;
struct v4l2_subdev;
struct v4l2_async_notifier;


enum v4l2_async_match_type {
	V4L2_ASYNC_MATCH_I2C,
	V4L2_ASYNC_MATCH_FWNODE,
};


struct v4l2_async_subdev {
	enum v4l2_async_match_type match_type;
	union {
		struct fwnode_handle *fwnode;
		struct {
			int adapter_id;
			unsigned short address;
		} i2c;
	} match;

	
	struct list_head list;
	struct list_head asd_list;
};


struct v4l2_async_notifier_operations {
	int (*bound)(struct v4l2_async_notifier *notifier,
		     struct v4l2_subdev *subdev,
		     struct v4l2_async_subdev *asd);
	int (*complete)(struct v4l2_async_notifier *notifier);
	void (*unbind)(struct v4l2_async_notifier *notifier,
		       struct v4l2_subdev *subdev,
		       struct v4l2_async_subdev *asd);
	void (*destroy)(struct v4l2_async_subdev *asd);
};


struct v4l2_async_notifier {
	const struct v4l2_async_notifier_operations *ops;
	struct v4l2_device *v4l2_dev;
	struct v4l2_subdev *sd;
	struct v4l2_async_notifier *parent;
	struct list_head asd_list;
	struct list_head waiting;
	struct list_head done;
	struct list_head list;
};


void v4l2_async_debug_init(struct dentry *debugfs_dir);


void v4l2_async_nf_init(struct v4l2_async_notifier *notifier);


int __v4l2_async_nf_add_subdev(struct v4l2_async_notifier *notifier,
			       struct v4l2_async_subdev *asd);

struct v4l2_async_subdev *
__v4l2_async_nf_add_fwnode(struct v4l2_async_notifier *notifier,
			   struct fwnode_handle *fwnode,
			   unsigned int asd_struct_size);

#define v4l2_async_nf_add_fwnode(notifier, fwnode, type)		\
	((type *)__v4l2_async_nf_add_fwnode(notifier, fwnode, sizeof(type)))

struct v4l2_async_subdev *
__v4l2_async_nf_add_fwnode_remote(struct v4l2_async_notifier *notif,
				  struct fwnode_handle *endpoint,
				  unsigned int asd_struct_size);

#define v4l2_async_nf_add_fwnode_remote(notifier, ep, type) \
	((type *)__v4l2_async_nf_add_fwnode_remote(notifier, ep, sizeof(type)))

struct v4l2_async_subdev *
__v4l2_async_nf_add_i2c(struct v4l2_async_notifier *notifier,
			int adapter_id, unsigned short address,
			unsigned int asd_struct_size);

#define v4l2_async_nf_add_i2c(notifier, adapter, address, type) \
	((type *)__v4l2_async_nf_add_i2c(notifier, adapter, address, \
					 sizeof(type)))


int v4l2_async_nf_register(struct v4l2_device *v4l2_dev,
			   struct v4l2_async_notifier *notifier);


int v4l2_async_subdev_nf_register(struct v4l2_subdev *sd,
				  struct v4l2_async_notifier *notifier);


void v4l2_async_nf_unregister(struct v4l2_async_notifier *notifier);


void v4l2_async_nf_cleanup(struct v4l2_async_notifier *notifier);


int v4l2_async_register_subdev(struct v4l2_subdev *sd);


int __must_check
v4l2_async_register_subdev_sensor(struct v4l2_subdev *sd);


void v4l2_async_unregister_subdev(struct v4l2_subdev *sd);
#endif
