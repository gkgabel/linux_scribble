/* SPDX-License-Identifier: (GPL-2.0+) */


#ifndef _I915_MEI_HDCP_INTERFACE_H_
#define _I915_MEI_HDCP_INTERFACE_H_

#include <linux/mutex.h>
#include <linux/device.h>
#include <drm/display/drm_hdcp.h>


enum hdcp_port_type {
	HDCP_PORT_TYPE_INVALID,
	HDCP_PORT_TYPE_INTEGRATED,
	HDCP_PORT_TYPE_LSPCON,
	HDCP_PORT_TYPE_CPDP
};


enum hdcp_wired_protocol {
	HDCP_PROTOCOL_INVALID,
	HDCP_PROTOCOL_HDMI,
	HDCP_PROTOCOL_DP
};

enum mei_fw_ddi {
	MEI_DDI_INVALID_PORT = 0x0,

	MEI_DDI_B = 1,
	MEI_DDI_C,
	MEI_DDI_D,
	MEI_DDI_E,
	MEI_DDI_F,
	MEI_DDI_A = 7,
	MEI_DDI_RANGE_END = MEI_DDI_A,
};


enum mei_fw_tc {
	MEI_INVALID_TRANSCODER = 0x00,
	MEI_TRANSCODER_EDP,
	MEI_TRANSCODER_DSI0,
	MEI_TRANSCODER_DSI1,
	MEI_TRANSCODER_A = 0x10,
	MEI_TRANSCODER_B,
	MEI_TRANSCODER_C,
	MEI_TRANSCODER_D
};


struct hdcp_port_data {
	enum mei_fw_ddi fw_ddi;
	enum mei_fw_tc fw_tc;
	u8 port_type;
	u8 protocol;
	u16 k;
	u32 seq_num_m;
	struct hdcp2_streamid_type *streams;
};


struct i915_hdcp_component_ops {
	
	struct module *owner;

	int (*initiate_hdcp2_session)(struct device *dev,
				      struct hdcp_port_data *data,
				      struct hdcp2_ake_init *ake_data);
	int (*verify_receiver_cert_prepare_km)(struct device *dev,
					       struct hdcp_port_data *data,
					       struct hdcp2_ake_send_cert
								*rx_cert,
					       bool *km_stored,
					       struct hdcp2_ake_no_stored_km
								*ek_pub_km,
					       size_t *msg_sz);
	int (*verify_hprime)(struct device *dev,
			     struct hdcp_port_data *data,
			     struct hdcp2_ake_send_hprime *rx_hprime);
	int (*store_pairing_info)(struct device *dev,
				  struct hdcp_port_data *data,
				  struct hdcp2_ake_send_pairing_info
								*pairing_info);
	int (*initiate_locality_check)(struct device *dev,
				       struct hdcp_port_data *data,
				       struct hdcp2_lc_init *lc_init_data);
	int (*verify_lprime)(struct device *dev,
			     struct hdcp_port_data *data,
			     struct hdcp2_lc_send_lprime *rx_lprime);
	int (*get_session_key)(struct device *dev,
			       struct hdcp_port_data *data,
			       struct hdcp2_ske_send_eks *ske_data);
	int (*repeater_check_flow_prepare_ack)(struct device *dev,
					       struct hdcp_port_data *data,
					       struct hdcp2_rep_send_receiverid_list
								*rep_topology,
					       struct hdcp2_rep_send_ack
								*rep_send_ack);
	int (*verify_mprime)(struct device *dev,
			     struct hdcp_port_data *data,
			     struct hdcp2_rep_stream_ready *stream_ready);
	int (*enable_hdcp_authentication)(struct device *dev,
					  struct hdcp_port_data *data);
	int (*close_hdcp_session)(struct device *dev,
				  struct hdcp_port_data *data);
};


struct i915_hdcp_comp_master {
	struct device *mei_dev;
	const struct i915_hdcp_component_ops *ops;

	
	struct mutex mutex;
};

#endif 
