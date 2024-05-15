/* SPDX-License-Identifier: GPL-2.0 */


#ifndef __THERMAL_H__
#define __THERMAL_H__

#include <linux/of.h>
#include <linux/idr.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/workqueue.h>
#include <uapi/linux/thermal.h>

#define THERMAL_MAX_TRIPS	12


#define THERMAL_CSTATE_INVALID -1UL


#define THERMAL_NO_LIMIT	((u32)~0)


#define THERMAL_WEIGHT_DEFAULT 0


#define THERMAL_TEMP_INVALID	-274000

struct thermal_zone_device;
struct thermal_cooling_device;
struct thermal_instance;
struct thermal_attr;

enum thermal_trend {
	THERMAL_TREND_STABLE, 
	THERMAL_TREND_RAISING, 
	THERMAL_TREND_DROPPING, 
};


enum thermal_notify_event {
	THERMAL_EVENT_UNSPECIFIED, 
	THERMAL_EVENT_TEMP_SAMPLE, 
	THERMAL_TRIP_VIOLATED, 
	THERMAL_TRIP_CHANGED, 
	THERMAL_DEVICE_DOWN, 
	THERMAL_DEVICE_UP, 
	THERMAL_DEVICE_POWER_CAPABILITY_CHANGED, 
	THERMAL_TABLE_CHANGED, 
	THERMAL_EVENT_KEEP_ALIVE, 
};

struct thermal_zone_device_ops {
	int (*bind) (struct thermal_zone_device *,
		     struct thermal_cooling_device *);
	int (*unbind) (struct thermal_zone_device *,
		       struct thermal_cooling_device *);
	int (*get_temp) (struct thermal_zone_device *, int *);
	int (*set_trips) (struct thermal_zone_device *, int, int);
	int (*change_mode) (struct thermal_zone_device *,
		enum thermal_device_mode);
	int (*get_trip_type) (struct thermal_zone_device *, int,
		enum thermal_trip_type *);
	int (*get_trip_temp) (struct thermal_zone_device *, int, int *);
	int (*set_trip_temp) (struct thermal_zone_device *, int, int);
	int (*get_trip_hyst) (struct thermal_zone_device *, int, int *);
	int (*set_trip_hyst) (struct thermal_zone_device *, int, int);
	int (*get_crit_temp) (struct thermal_zone_device *, int *);
	int (*set_emul_temp) (struct thermal_zone_device *, int);
	int (*get_trend) (struct thermal_zone_device *, int,
			  enum thermal_trend *);
	void (*hot)(struct thermal_zone_device *);
	void (*critical)(struct thermal_zone_device *);
};


struct thermal_trip {
	int temperature;
	int hysteresis;
	enum thermal_trip_type type;
};

struct thermal_cooling_device_ops {
	int (*get_max_state) (struct thermal_cooling_device *, unsigned long *);
	int (*get_cur_state) (struct thermal_cooling_device *, unsigned long *);
	int (*set_cur_state) (struct thermal_cooling_device *, unsigned long);
	int (*get_requested_power)(struct thermal_cooling_device *, u32 *);
	int (*state2power)(struct thermal_cooling_device *, unsigned long, u32 *);
	int (*power2state)(struct thermal_cooling_device *, u32, unsigned long *);
};

struct thermal_cooling_device {
	int id;
	char *type;
	struct device device;
	struct device_node *np;
	void *devdata;
	void *stats;
	const struct thermal_cooling_device_ops *ops;
	bool updated; 
	struct mutex lock; 
	struct list_head thermal_instances;
	struct list_head node;
};


struct thermal_zone_device {
	int id;
	char type[THERMAL_NAME_LENGTH];
	struct device device;
	struct attribute_group trips_attribute_group;
	struct thermal_attr *trip_temp_attrs;
	struct thermal_attr *trip_type_attrs;
	struct thermal_attr *trip_hyst_attrs;
	enum thermal_device_mode mode;
	void *devdata;
	struct thermal_trip *trips;
	int num_trips;
	unsigned long trips_disabled;	
	unsigned long passive_delay_jiffies;
	unsigned long polling_delay_jiffies;
	int temperature;
	int last_temperature;
	int emul_temperature;
	int passive;
	int prev_low_trip;
	int prev_high_trip;
	atomic_t need_update;
	struct thermal_zone_device_ops *ops;
	struct thermal_zone_params *tzp;
	struct thermal_governor *governor;
	void *governor_data;
	struct list_head thermal_instances;
	struct ida ida;
	struct mutex lock;
	struct list_head node;
	struct delayed_work poll_queue;
	enum thermal_notify_event notify_event;
};


struct thermal_governor {
	char name[THERMAL_NAME_LENGTH];
	int (*bind_to_tz)(struct thermal_zone_device *tz);
	void (*unbind_from_tz)(struct thermal_zone_device *tz);
	int (*throttle)(struct thermal_zone_device *tz, int trip);
	struct list_head	governor_list;
};


struct thermal_bind_params {
	struct thermal_cooling_device *cdev;

	
	int weight;

	
	int trip_mask;

	
	unsigned long *binding_limits;
	int (*match) (struct thermal_zone_device *tz,
			struct thermal_cooling_device *cdev);
};


struct thermal_zone_params {
	char governor_name[THERMAL_NAME_LENGTH];

	
	bool no_hwmon;

	int num_tbps;	
	struct thermal_bind_params *tbp;

	
	u32 sustainable_power;

	
	s32 k_po;

	
	s32 k_pu;

	
	s32 k_i;

	
	s32 k_d;

	
	s32 integral_cutoff;

	
	int slope;
	
	int offset;
};


struct thermal_zone_of_device_ops {
	int (*get_temp)(void *, int *);
	int (*get_trend)(void *, int, enum thermal_trend *);
	int (*set_trips)(void *, int, int);
	int (*set_emul_temp)(void *, int);
	int (*set_trip_temp)(void *, int, int);
	int (*change_mode) (void *, enum thermal_device_mode);
};


#ifdef CONFIG_THERMAL_OF
int thermal_zone_of_get_sensor_id(struct device_node *tz_np,
				  struct device_node *sensor_np,
				  u32 *id);
struct thermal_zone_device *
thermal_zone_of_sensor_register(struct device *dev, int id, void *data,
				const struct thermal_zone_of_device_ops *ops);
void thermal_zone_of_sensor_unregister(struct device *dev,
				       struct thermal_zone_device *tz);
struct thermal_zone_device *devm_thermal_zone_of_sensor_register(
		struct device *dev, int id, void *data,
		const struct thermal_zone_of_device_ops *ops);
void devm_thermal_zone_of_sensor_unregister(struct device *dev,
					    struct thermal_zone_device *tz);
#else

static inline int thermal_zone_of_get_sensor_id(struct device_node *tz_np,
					 struct device_node *sensor_np,
					 u32 *id)
{
	return -ENOENT;
}
static inline struct thermal_zone_device *
thermal_zone_of_sensor_register(struct device *dev, int id, void *data,
				const struct thermal_zone_of_device_ops *ops)
{
	return ERR_PTR(-ENODEV);
}

static inline
void thermal_zone_of_sensor_unregister(struct device *dev,
				       struct thermal_zone_device *tz)
{
}

static inline struct thermal_zone_device *devm_thermal_zone_of_sensor_register(
		struct device *dev, int id, void *data,
		const struct thermal_zone_of_device_ops *ops)
{
	return ERR_PTR(-ENODEV);
}

static inline
void devm_thermal_zone_of_sensor_unregister(struct device *dev,
					    struct thermal_zone_device *tz)
{
}

#endif

#ifdef CONFIG_THERMAL
struct thermal_zone_device *thermal_zone_device_register(const char *, int, int,
		void *, struct thermal_zone_device_ops *,
		struct thermal_zone_params *, int, int);

void thermal_zone_device_unregister(struct thermal_zone_device *);

struct thermal_zone_device *
thermal_zone_device_register_with_trips(const char *, struct thermal_trip *, int, int,
					void *, struct thermal_zone_device_ops *,
					struct thermal_zone_params *, int, int);

int thermal_zone_bind_cooling_device(struct thermal_zone_device *, int,
				     struct thermal_cooling_device *,
				     unsigned long, unsigned long,
				     unsigned int);
int thermal_zone_unbind_cooling_device(struct thermal_zone_device *, int,
				       struct thermal_cooling_device *);
void thermal_zone_device_update(struct thermal_zone_device *,
				enum thermal_notify_event);

struct thermal_cooling_device *thermal_cooling_device_register(const char *,
		void *, const struct thermal_cooling_device_ops *);
struct thermal_cooling_device *
thermal_of_cooling_device_register(struct device_node *np, const char *, void *,
				   const struct thermal_cooling_device_ops *);
struct thermal_cooling_device *
devm_thermal_of_cooling_device_register(struct device *dev,
				struct device_node *np,
				char *type, void *devdata,
				const struct thermal_cooling_device_ops *ops);
void thermal_cooling_device_unregister(struct thermal_cooling_device *);
struct thermal_zone_device *thermal_zone_get_zone_by_name(const char *name);
int thermal_zone_get_temp(struct thermal_zone_device *tz, int *temp);
int thermal_zone_get_slope(struct thermal_zone_device *tz);
int thermal_zone_get_offset(struct thermal_zone_device *tz);

int thermal_zone_device_enable(struct thermal_zone_device *tz);
int thermal_zone_device_disable(struct thermal_zone_device *tz);
void thermal_zone_device_critical(struct thermal_zone_device *tz);
#else
static inline struct thermal_zone_device *thermal_zone_device_register(
	const char *type, int trips, int mask, void *devdata,
	struct thermal_zone_device_ops *ops,
	struct thermal_zone_params *tzp,
	int passive_delay, int polling_delay)
{ return ERR_PTR(-ENODEV); }
static inline void thermal_zone_device_unregister(
	struct thermal_zone_device *tz)
{ }
static inline struct thermal_cooling_device *
thermal_cooling_device_register(const char *type, void *devdata,
	const struct thermal_cooling_device_ops *ops)
{ return ERR_PTR(-ENODEV); }
static inline struct thermal_cooling_device *
thermal_of_cooling_device_register(struct device_node *np,
	const char *type, void *devdata,
	const struct thermal_cooling_device_ops *ops)
{ return ERR_PTR(-ENODEV); }
static inline struct thermal_cooling_device *
devm_thermal_of_cooling_device_register(struct device *dev,
				struct device_node *np,
				char *type, void *devdata,
				const struct thermal_cooling_device_ops *ops)
{
	return ERR_PTR(-ENODEV);
}
static inline void thermal_cooling_device_unregister(
	struct thermal_cooling_device *cdev)
{ }
static inline struct thermal_zone_device *thermal_zone_get_zone_by_name(
		const char *name)
{ return ERR_PTR(-ENODEV); }
static inline int thermal_zone_get_temp(
		struct thermal_zone_device *tz, int *temp)
{ return -ENODEV; }
static inline int thermal_zone_get_slope(
		struct thermal_zone_device *tz)
{ return -ENODEV; }
static inline int thermal_zone_get_offset(
		struct thermal_zone_device *tz)
{ return -ENODEV; }

static inline int thermal_zone_device_enable(struct thermal_zone_device *tz)
{ return -ENODEV; }

static inline int thermal_zone_device_disable(struct thermal_zone_device *tz)
{ return -ENODEV; }
#endif 

#endif 
