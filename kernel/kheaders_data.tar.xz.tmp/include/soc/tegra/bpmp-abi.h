/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef ABI_BPMP_ABI_H
#define ABI_BPMP_ABI_H

#if defined(LK) || defined(BPMP_ABI_HAVE_STDC)
#include <stddef.h>
#include <stdint.h>
#endif

#ifndef BPMP_ABI_PACKED
#ifdef __ABI_PACKED
#define BPMP_ABI_PACKED __ABI_PACKED
#else
#define BPMP_ABI_PACKED __attribute__((packed))
#endif
#endif

#ifdef NO_GCC_EXTENSIONS
#define BPMP_ABI_EMPTY char empty;
#define BPMP_ABI_EMPTY_ARRAY 1
#else
#define BPMP_ABI_EMPTY
#define BPMP_ABI_EMPTY_ARRAY 0
#endif

#ifndef BPMP_UNION_ANON
#ifdef __UNION_ANON
#define BPMP_UNION_ANON __UNION_ANON
#else
#define BPMP_UNION_ANON
#endif
#endif








struct mrq_request {
	
	uint32_t mrq;

	
	uint32_t flags;
} BPMP_ABI_PACKED;


struct mrq_response {
	
	int32_t err;
	
	uint32_t flags;
} BPMP_ABI_PACKED;


#define MSG_MIN_SZ	128U

#define MSG_DATA_MIN_SZ	120U



#define MRQ_PING		0U
#define MRQ_QUERY_TAG		1U
#define MRQ_MODULE_LOAD		4U
#define MRQ_MODULE_UNLOAD	5U
#define MRQ_TRACE_MODIFY	7U
#define MRQ_WRITE_TRACE		8U
#define MRQ_THREADED_PING	9U
#define MRQ_MODULE_MAIL		11U
#define MRQ_DEBUGFS		19U
#define MRQ_RESET		20U
#define MRQ_I2C			21U
#define MRQ_CLK			22U
#define MRQ_QUERY_ABI		23U
#define MRQ_PG_READ_STATE	25U
#define MRQ_PG_UPDATE_STATE	26U
#define MRQ_THERMAL		27U
#define MRQ_CPU_VHINT		28U
#define MRQ_ABI_RATCHET		29U
#define MRQ_EMC_DVFS_LATENCY	31U
#define MRQ_TRACE_ITER		64U
#define MRQ_RINGBUF_CONSOLE	65U
#define MRQ_PG			66U
#define MRQ_CPU_NDIV_LIMITS	67U
#define MRQ_STRAP               68U
#define MRQ_UPHY		69U
#define MRQ_CPU_AUTO_CC3	70U
#define MRQ_QUERY_FW_TAG	71U
#define MRQ_FMON		72U
#define MRQ_EC			73U
#define MRQ_DEBUG		75U




#define MAX_CPU_MRQ_ID		75U






struct mrq_ping_request {

	uint32_t challenge;
} BPMP_ABI_PACKED;


struct mrq_ping_response {
	
	uint32_t reply;
} BPMP_ABI_PACKED;




struct mrq_query_tag_request {
  
	uint32_t addr;
} BPMP_ABI_PACKED;





struct mrq_query_fw_tag_response {
  
	uint8_t tag[32];
} BPMP_ABI_PACKED;




struct mrq_module_load_request {
	
	uint32_t phys_addr;
	
	uint32_t size;
} BPMP_ABI_PACKED;


struct mrq_module_load_response {
	
	uint32_t base;
} BPMP_ABI_PACKED;





struct mrq_module_unload_request {
	
	uint32_t base;
} BPMP_ABI_PACKED;





struct mrq_trace_modify_request {
	
	uint32_t clr;
	
	uint32_t set;
} BPMP_ABI_PACKED;


struct mrq_trace_modify_response {
	
	uint32_t mask;
} BPMP_ABI_PACKED;




struct mrq_write_trace_request {
	
	uint32_t area;
	
	uint32_t size;
} BPMP_ABI_PACKED;


struct mrq_write_trace_response {
	
	uint32_t eof;
} BPMP_ABI_PACKED;


struct mrq_threaded_ping_request {
	uint32_t challenge;
} BPMP_ABI_PACKED;


struct mrq_threaded_ping_response {
	uint32_t reply;
} BPMP_ABI_PACKED;




struct mrq_module_mail_request {
	
	uint32_t base;
	
	uint8_t data[BPMP_ABI_EMPTY_ARRAY];
} BPMP_ABI_PACKED;


struct mrq_module_mail_response {
	
	uint8_t data[BPMP_ABI_EMPTY_ARRAY];
} BPMP_ABI_PACKED;







enum mrq_debugfs_commands {
	
	CMD_DEBUGFS_READ = 1,
	
	CMD_DEBUGFS_WRITE = 2,
	
	CMD_DEBUGFS_DUMPDIR = 3,
	
	CMD_DEBUGFS_MAX
};


struct cmd_debugfs_fileop_request {
	
	uint32_t fnameaddr;
	
	uint32_t fnamelen;
	
	uint32_t dataaddr;
	
	uint32_t datalen;
} BPMP_ABI_PACKED;


struct cmd_debugfs_dumpdir_request {
	
	uint32_t dataaddr;
	
	uint32_t datalen;
} BPMP_ABI_PACKED;


struct cmd_debugfs_fileop_response {
	
	uint32_t reserved;
	
	uint32_t nbytes;
} BPMP_ABI_PACKED;


struct cmd_debugfs_dumpdir_response {
	
	uint32_t reserved;
	
	uint32_t nbytes;
} BPMP_ABI_PACKED;


struct mrq_debugfs_request {
	
	uint32_t cmd;
	union {
		struct cmd_debugfs_fileop_request fop;
		struct cmd_debugfs_dumpdir_request dumpdir;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;


struct mrq_debugfs_response {
	
	int32_t reserved;
	union {
		
		struct cmd_debugfs_fileop_response fop;
		
		struct cmd_debugfs_dumpdir_response dumpdir;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;


#define DEBUGFS_S_ISDIR	(1 << 9)
#define DEBUGFS_S_IRUSR	(1 << 8)
#define DEBUGFS_S_IWUSR	(1 << 7)





enum mrq_debug_commands {
	
	CMD_DEBUG_OPEN_RO = 0,
	
	CMD_DEBUG_OPEN_WO = 1,
	
	CMD_DEBUG_READ = 2,
	
	CMD_DEBUG_WRITE = 3,
	
	CMD_DEBUG_CLOSE = 4,
	
	CMD_DEBUG_MAX
};


#define DEBUG_MAX_OPEN_FILES	1


#define DEBUG_FNAME_MAX_SZ	(MSG_DATA_MIN_SZ - 4)


struct cmd_debug_fopen_request {
	
	char name[DEBUG_FNAME_MAX_SZ];
} BPMP_ABI_PACKED;


struct cmd_debug_fopen_response {
	
	uint32_t fd;
	
	uint32_t datalen;
} BPMP_ABI_PACKED;


struct cmd_debug_fread_request {
	
	uint32_t fd;
} BPMP_ABI_PACKED;


#define DEBUG_READ_MAX_SZ	(MSG_DATA_MIN_SZ - 4)


struct cmd_debug_fread_response {
	
	uint32_t readlen;
	
	char data[DEBUG_READ_MAX_SZ];
} BPMP_ABI_PACKED;


#define DEBUG_WRITE_MAX_SZ	(MSG_DATA_MIN_SZ - 12)


struct cmd_debug_fwrite_request {
	
	uint32_t fd;
	
	uint32_t datalen;
	
	char data[DEBUG_WRITE_MAX_SZ];
} BPMP_ABI_PACKED;


struct cmd_debug_fclose_request {
	
	uint32_t fd;
} BPMP_ABI_PACKED;


struct mrq_debug_request {
	
	uint32_t cmd;
	union {
		
		struct cmd_debug_fopen_request fop;
		
		struct cmd_debug_fread_request frd;
		
		struct cmd_debug_fwrite_request fwr;
		
		struct cmd_debug_fclose_request fcl;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;


struct mrq_debug_response {
	union {
		
		struct cmd_debug_fopen_response fop;
		
		struct cmd_debug_fread_response frd;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;



enum mrq_reset_commands {
	
	CMD_RESET_ASSERT = 1,
	
	CMD_RESET_DEASSERT = 2,
	
	CMD_RESET_MODULE = 3,
	
	CMD_RESET_GET_MAX_ID = 4,

	
	CMD_RESET_MAX,
};


struct mrq_reset_request {
	
	uint32_t cmd;
	
	uint32_t reset_id;
} BPMP_ABI_PACKED;


struct cmd_reset_get_max_id_response {
	
	uint32_t max_id;
} BPMP_ABI_PACKED;


struct mrq_reset_response {
	union {
		struct cmd_reset_get_max_id_response reset_get_max_id;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;




#define TEGRA_I2C_IPC_MAX_IN_BUF_SIZE	(MSG_DATA_MIN_SZ - 12U)
#define TEGRA_I2C_IPC_MAX_OUT_BUF_SIZE	(MSG_DATA_MIN_SZ - 4U)

#define SERIALI2C_TEN           0x0010U
#define SERIALI2C_RD            0x0001U
#define SERIALI2C_STOP          0x8000U
#define SERIALI2C_NOSTART       0x4000U
#define SERIALI2C_REV_DIR_ADDR  0x2000U
#define SERIALI2C_IGNORE_NAK    0x1000U
#define SERIALI2C_NO_RD_ACK     0x0800U
#define SERIALI2C_RECV_LEN      0x0400U

enum {
	CMD_I2C_XFER = 1
};


struct serial_i2c_request {
	
	uint16_t addr;
	
	uint16_t flags;
	
	uint16_t len;
	
	uint8_t data[];
} BPMP_ABI_PACKED;


struct cmd_i2c_xfer_request {
	
	uint32_t bus_id;

	
	uint32_t data_size;

	
	uint8_t data_buf[TEGRA_I2C_IPC_MAX_IN_BUF_SIZE];
} BPMP_ABI_PACKED;


struct cmd_i2c_xfer_response {
	
	uint32_t data_size;
	
	uint8_t data_buf[TEGRA_I2C_IPC_MAX_OUT_BUF_SIZE];
} BPMP_ABI_PACKED;


struct mrq_i2c_request {
	
	uint32_t cmd;
	
	struct cmd_i2c_xfer_request xfer;
} BPMP_ABI_PACKED;


struct mrq_i2c_response {
	struct cmd_i2c_xfer_response xfer;
} BPMP_ABI_PACKED;




enum {
	CMD_CLK_GET_RATE = 1,
	CMD_CLK_SET_RATE = 2,
	CMD_CLK_ROUND_RATE = 3,
	CMD_CLK_GET_PARENT = 4,
	CMD_CLK_SET_PARENT = 5,
	CMD_CLK_IS_ENABLED = 6,
	CMD_CLK_ENABLE = 7,
	CMD_CLK_DISABLE = 8,
	CMD_CLK_GET_ALL_INFO = 14,
	CMD_CLK_GET_MAX_CLK_ID = 15,
	CMD_CLK_GET_FMAX_AT_VMIN = 16,
	CMD_CLK_MAX,
};

#define BPMP_CLK_HAS_MUX	(1U << 0U)
#define BPMP_CLK_HAS_SET_RATE	(1U << 1U)
#define BPMP_CLK_IS_ROOT	(1U << 2U)
#define BPMP_CLK_IS_VAR_ROOT	(1U << 3U)

#define MRQ_CLK_NAME_MAXLEN	40U
#define MRQ_CLK_MAX_PARENTS	16U


struct cmd_clk_get_rate_request {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;

struct cmd_clk_get_rate_response {
	int64_t rate;
} BPMP_ABI_PACKED;

struct cmd_clk_set_rate_request {
	int32_t unused;
	int64_t rate;
} BPMP_ABI_PACKED;

struct cmd_clk_set_rate_response {
	int64_t rate;
} BPMP_ABI_PACKED;

struct cmd_clk_round_rate_request {
	int32_t unused;
	int64_t rate;
} BPMP_ABI_PACKED;

struct cmd_clk_round_rate_response {
	int64_t rate;
} BPMP_ABI_PACKED;


struct cmd_clk_get_parent_request {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;

struct cmd_clk_get_parent_response {
	uint32_t parent_id;
} BPMP_ABI_PACKED;

struct cmd_clk_set_parent_request {
	uint32_t parent_id;
} BPMP_ABI_PACKED;

struct cmd_clk_set_parent_response {
	uint32_t parent_id;
} BPMP_ABI_PACKED;


struct cmd_clk_is_enabled_request {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;


struct cmd_clk_is_enabled_response {
	
	int32_t state;
} BPMP_ABI_PACKED;


struct cmd_clk_enable_request {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;


struct cmd_clk_enable_response {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;


struct cmd_clk_disable_request {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;


struct cmd_clk_disable_response {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;


struct cmd_clk_get_all_info_request {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;

struct cmd_clk_get_all_info_response {
	uint32_t flags;
	uint32_t parent;
	uint32_t parents[MRQ_CLK_MAX_PARENTS];
	uint8_t num_parents;
	uint8_t name[MRQ_CLK_NAME_MAXLEN];
} BPMP_ABI_PACKED;


struct cmd_clk_get_max_clk_id_request {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;

struct cmd_clk_get_max_clk_id_response {
	uint32_t max_id;
} BPMP_ABI_PACKED;


struct cmd_clk_get_fmax_at_vmin_request {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;

struct cmd_clk_get_fmax_at_vmin_response {
	int64_t rate;
} BPMP_ABI_PACKED;



struct mrq_clk_request {
	
	uint32_t cmd_and_id;

	union {
		
		struct cmd_clk_get_rate_request clk_get_rate;
		struct cmd_clk_set_rate_request clk_set_rate;
		struct cmd_clk_round_rate_request clk_round_rate;
		
		struct cmd_clk_get_parent_request clk_get_parent;
		struct cmd_clk_set_parent_request clk_set_parent;
		
		struct cmd_clk_enable_request clk_enable;
		
		struct cmd_clk_disable_request clk_disable;
		
		struct cmd_clk_is_enabled_request clk_is_enabled;
		
		struct cmd_clk_get_all_info_request clk_get_all_info;
		
		struct cmd_clk_get_max_clk_id_request clk_get_max_clk_id;
		
		struct cmd_clk_get_fmax_at_vmin_request clk_get_fmax_at_vmin;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;



struct mrq_clk_response {
	union {
		struct cmd_clk_get_rate_response clk_get_rate;
		struct cmd_clk_set_rate_response clk_set_rate;
		struct cmd_clk_round_rate_response clk_round_rate;
		struct cmd_clk_get_parent_response clk_get_parent;
		struct cmd_clk_set_parent_response clk_set_parent;
		
		struct cmd_clk_enable_response clk_enable;
		
		struct cmd_clk_disable_response clk_disable;
		struct cmd_clk_is_enabled_response clk_is_enabled;
		struct cmd_clk_get_all_info_response clk_get_all_info;
		struct cmd_clk_get_max_clk_id_response clk_get_max_clk_id;
		struct cmd_clk_get_fmax_at_vmin_response clk_get_fmax_at_vmin;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;






struct mrq_query_abi_request {
	
	uint32_t mrq;
} BPMP_ABI_PACKED;


struct mrq_query_abi_response {
	
	int32_t status;
} BPMP_ABI_PACKED;




struct mrq_pg_read_state_request {
	
	uint32_t partition_id;
} BPMP_ABI_PACKED;


struct mrq_pg_read_state_response {
	
	uint32_t sram_state;
	
	uint32_t logic_state;
} BPMP_ABI_PACKED;






struct mrq_pg_update_state_request {
	
	uint32_t partition_id;
	
	uint32_t sram_state;
	
	uint32_t logic_state;
	
	uint32_t clock_state;
} BPMP_ABI_PACKED;



enum mrq_pg_cmd {
	
	CMD_PG_QUERY_ABI = 0,

	
	CMD_PG_SET_STATE = 1,

	
	CMD_PG_GET_STATE = 2,

	
	CMD_PG_GET_NAME = 3,


	
	CMD_PG_GET_MAX_ID = 4,
};

#define MRQ_PG_NAME_MAXLEN	40

enum pg_states {
	
	PG_STATE_OFF = 0,
	
	PG_STATE_ON = 1,
	
	PG_STATE_RUNNING = 2,
};

struct cmd_pg_query_abi_request {
	
	uint32_t type;
} BPMP_ABI_PACKED;

struct cmd_pg_set_state_request {
	
	uint32_t state;
} BPMP_ABI_PACKED;


struct cmd_pg_get_state_response {
	
	uint32_t state;
} BPMP_ABI_PACKED;

struct cmd_pg_get_name_response {
	uint8_t name[MRQ_PG_NAME_MAXLEN];
} BPMP_ABI_PACKED;

struct cmd_pg_get_max_id_response {
	uint32_t max_id;
} BPMP_ABI_PACKED;


struct mrq_pg_request {
	uint32_t cmd;
	uint32_t id;
	union {
		struct cmd_pg_query_abi_request query_abi;
		struct cmd_pg_set_state_request set_state;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;


struct mrq_pg_response {
	union {
		struct cmd_pg_get_state_response get_state;
		struct cmd_pg_get_name_response get_name;
		struct cmd_pg_get_max_id_response get_max_id;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;




enum mrq_thermal_host_to_bpmp_cmd {
	
	CMD_THERMAL_QUERY_ABI = 0,

	
	CMD_THERMAL_GET_TEMP = 1,

	
	CMD_THERMAL_SET_TRIP = 2,

	
	CMD_THERMAL_GET_NUM_ZONES = 3,

	
	CMD_THERMAL_GET_THERMTRIP = 4,

	
	CMD_THERMAL_HOST_TO_BPMP_NUM
};

enum mrq_thermal_bpmp_to_host_cmd {
	
	CMD_THERMAL_HOST_TRIP_REACHED = 100,

	
	CMD_THERMAL_BPMP_TO_HOST_NUM
};


struct cmd_thermal_query_abi_request {
	uint32_t type;
} BPMP_ABI_PACKED;


struct cmd_thermal_get_temp_request {
	uint32_t zone;
} BPMP_ABI_PACKED;


struct cmd_thermal_get_temp_response {
	int32_t temp;
} BPMP_ABI_PACKED;


struct cmd_thermal_set_trip_request {
	uint32_t zone;
	int32_t low;
	int32_t high;
	uint32_t enabled;
} BPMP_ABI_PACKED;


struct cmd_thermal_host_trip_reached_request {
	uint32_t zone;
} BPMP_ABI_PACKED;


struct cmd_thermal_get_num_zones_response {
	uint32_t num;
} BPMP_ABI_PACKED;


struct cmd_thermal_get_thermtrip_request {
	uint32_t zone;
} BPMP_ABI_PACKED;


struct cmd_thermal_get_thermtrip_response {
	int32_t thermtrip;
} BPMP_ABI_PACKED;


struct mrq_thermal_host_to_bpmp_request {
	uint32_t type;
	union {
		struct cmd_thermal_query_abi_request query_abi;
		struct cmd_thermal_get_temp_request get_temp;
		struct cmd_thermal_set_trip_request set_trip;
		struct cmd_thermal_get_thermtrip_request get_thermtrip;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;


struct mrq_thermal_bpmp_to_host_request {
	uint32_t type;
	union {
		struct cmd_thermal_host_trip_reached_request host_trip_reached;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;


union mrq_thermal_bpmp_to_host_response {
	struct cmd_thermal_get_temp_response get_temp;
	struct cmd_thermal_get_thermtrip_response get_thermtrip;
	struct cmd_thermal_get_num_zones_response get_num_zones;
} BPMP_ABI_PACKED;





struct mrq_cpu_vhint_request {
	
	uint32_t addr;
	
	uint32_t cluster_id;
} BPMP_ABI_PACKED;


struct cpu_vhint_data {
	uint32_t ref_clk_hz; 
	uint16_t pdiv; 
	uint16_t mdiv; 
	uint16_t ndiv_max; 
	
	uint16_t ndiv[80];
	
	uint16_t ndiv_min;
	
	uint16_t vfloor;
	
	uint16_t vceil;
	
	uint16_t vindex_mult;
	
	uint16_t vindex_div;
	
	uint16_t reserved[328];
} BPMP_ABI_PACKED;






#define BPMP_ABI_RATCHET_VALUE 3


struct mrq_abi_ratchet_request {
	
	uint16_t ratchet;
};


struct mrq_abi_ratchet_response {
	
	uint16_t ratchet;
};





struct emc_dvfs_latency {
	
	uint32_t freq;
	
	uint32_t latency;
} BPMP_ABI_PACKED;

#define EMC_DVFS_LATENCY_MAX_SIZE	14

struct mrq_emc_dvfs_latency_response {
	
	uint32_t num_pairs;
	
	struct emc_dvfs_latency pairs[EMC_DVFS_LATENCY_MAX_SIZE];
} BPMP_ABI_PACKED;






struct mrq_cpu_ndiv_limits_request {
	
	uint32_t cluster_id;
} BPMP_ABI_PACKED;


struct mrq_cpu_ndiv_limits_response {
	
	uint32_t ref_clk_hz;
	
	uint16_t pdiv;
	
	uint16_t mdiv;
	
	uint16_t ndiv_max;
	
	uint16_t ndiv_min;
} BPMP_ABI_PACKED;







struct mrq_cpu_auto_cc3_request {
	
	uint32_t cluster_id;
} BPMP_ABI_PACKED;


struct mrq_cpu_auto_cc3_response {
	
	uint32_t auto_cc3_config;
} BPMP_ABI_PACKED;





enum {
	
	TRACE_ITER_INIT = 0,
	
	TRACE_ITER_CLEAN = 1
};


struct mrq_trace_iter_request {
	
	uint32_t cmd;
} BPMP_ABI_PACKED;






#define MRQ_RINGBUF_CONSOLE_MAX_WRITE_LEN 112


#define MRQ_RINGBUF_CONSOLE_MAX_READ_LEN 116

enum mrq_ringbuf_console_host_to_bpmp_cmd {
	
	CMD_RINGBUF_CONSOLE_QUERY_ABI = 0,
	
	CMD_RINGBUF_CONSOLE_READ = 1,
	
	CMD_RINGBUF_CONSOLE_WRITE = 2,
	
	CMD_RINGBUF_CONSOLE_GET_FIFO = 3,
};


struct cmd_ringbuf_console_query_abi_req {
	
	uint32_t cmd;
} BPMP_ABI_PACKED;


struct cmd_ringbuf_console_query_abi_resp {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;


struct cmd_ringbuf_console_read_req {
	
	uint8_t len;
} BPMP_ABI_PACKED;


struct cmd_ringbuf_console_read_resp {
	
	uint8_t data[MRQ_RINGBUF_CONSOLE_MAX_READ_LEN];
	
	uint8_t len;
} BPMP_ABI_PACKED;


struct cmd_ringbuf_console_write_req {
	
	uint8_t data[MRQ_RINGBUF_CONSOLE_MAX_WRITE_LEN];
	
	uint8_t len;
} BPMP_ABI_PACKED;


struct cmd_ringbuf_console_write_resp {
	
	uint32_t space_avail;
	
	uint8_t len;
} BPMP_ABI_PACKED;


struct cmd_ringbuf_console_get_fifo_req {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;


struct cmd_ringbuf_console_get_fifo_resp {
	
	uint64_t bpmp_tx_buf_addr;
	
	uint64_t bpmp_tx_head_addr;
	
	uint64_t bpmp_tx_tail_addr;
	
	uint32_t bpmp_tx_buf_len;
} BPMP_ABI_PACKED;


struct mrq_ringbuf_console_host_to_bpmp_request {
	
	uint32_t type;
	
	union {
		struct cmd_ringbuf_console_query_abi_req query_abi;
		struct cmd_ringbuf_console_read_req read;
		struct cmd_ringbuf_console_write_req write;
		struct cmd_ringbuf_console_get_fifo_req get_fifo;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;


union mrq_ringbuf_console_bpmp_to_host_response {
	struct cmd_ringbuf_console_query_abi_resp query_abi;
	struct cmd_ringbuf_console_read_resp read;
	struct cmd_ringbuf_console_write_resp write;
	struct cmd_ringbuf_console_get_fifo_resp get_fifo;
} BPMP_ABI_PACKED;



enum mrq_strap_cmd {
	
	STRAP_RESERVED = 0,
	
	STRAP_SET = 1
};


struct mrq_strap_request {
	
	uint32_t cmd;
	
	uint32_t id;
	
	uint32_t value;
} BPMP_ABI_PACKED;





enum {
	CMD_UPHY_PCIE_LANE_MARGIN_CONTROL = 1,
	CMD_UPHY_PCIE_LANE_MARGIN_STATUS = 2,
	CMD_UPHY_PCIE_EP_CONTROLLER_PLL_INIT = 3,
	CMD_UPHY_PCIE_CONTROLLER_STATE = 4,
	CMD_UPHY_PCIE_EP_CONTROLLER_PLL_OFF = 5,
	CMD_UPHY_MAX,
};

struct cmd_uphy_margin_control_request {
	
	int32_t en;
	
	int32_t clr;
	
	uint32_t x;
	
	uint32_t y;
	
	uint32_t nblks;
} BPMP_ABI_PACKED;

struct cmd_uphy_margin_status_response {
	
	uint32_t status;
} BPMP_ABI_PACKED;

struct cmd_uphy_ep_controller_pll_init_request {
	
	uint8_t ep_controller;
} BPMP_ABI_PACKED;

struct cmd_uphy_pcie_controller_state_request {
	
	uint8_t pcie_controller;
	uint8_t enable;
} BPMP_ABI_PACKED;

struct cmd_uphy_ep_controller_pll_off_request {
	
	uint8_t ep_controller;
} BPMP_ABI_PACKED;



struct mrq_uphy_request {
	
	uint16_t lane;
	
	uint16_t cmd;

	union {
		struct cmd_uphy_margin_control_request uphy_set_margin_control;
		struct cmd_uphy_ep_controller_pll_init_request ep_ctrlr_pll_init;
		struct cmd_uphy_pcie_controller_state_request controller_state;
		struct cmd_uphy_ep_controller_pll_off_request ep_ctrlr_pll_off;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;



struct mrq_uphy_response {
	union {
		struct cmd_uphy_margin_status_response uphy_get_margin_status;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;





enum {
	
	CMD_FMON_GEAR_CLAMP = 1,
	
	CMD_FMON_GEAR_FREE = 2,
	
	CMD_FMON_GEAR_GET = 3,
	CMD_FMON_NUM,
};

struct cmd_fmon_gear_clamp_request {
	int32_t unused;
	int64_t rate;
} BPMP_ABI_PACKED;


struct cmd_fmon_gear_clamp_response {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;


struct cmd_fmon_gear_free_request {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;


struct cmd_fmon_gear_free_response {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;


struct cmd_fmon_gear_get_request {
	BPMP_ABI_EMPTY
} BPMP_ABI_PACKED;

struct cmd_fmon_gear_get_response {
	int64_t rate;
} BPMP_ABI_PACKED;



struct mrq_fmon_request {
	
	uint32_t cmd_and_id;

	union {
		struct cmd_fmon_gear_clamp_request fmon_gear_clamp;
		
		struct cmd_fmon_gear_free_request fmon_gear_free;
		
		struct cmd_fmon_gear_get_request fmon_gear_get;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;



struct mrq_fmon_response {
	union {
		
		struct cmd_fmon_gear_clamp_response fmon_gear_clamp;
		
		struct cmd_fmon_gear_free_response fmon_gear_free;
		struct cmd_fmon_gear_get_response fmon_gear_get;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;





enum {
	
	CMD_EC_STATUS_GET = 1,	

	
	CMD_EC_STATUS_EX_GET = 2,
	CMD_EC_NUM,
};


enum bpmp_ec_err_type {
	
	EC_ERR_TYPE_PARITY_INTERNAL		= 1,

	
	EC_ERR_TYPE_ECC_SEC_INTERNAL		= 2,

	
	EC_ERR_TYPE_ECC_DED_INTERNAL		= 3,

	
	EC_ERR_TYPE_COMPARATOR			= 4,

	
	EC_ERR_TYPE_REGISTER_PARITY		= 5,

	
	EC_ERR_TYPE_PARITY_SRAM			= 6,

	
	EC_ERR_TYPE_CLOCK_MONITOR		= 9,

	
	EC_ERR_TYPE_VOLTAGE_MONITOR		= 10,

	
	EC_ERR_TYPE_SW_CORRECTABLE		= 16,

	
	EC_ERR_TYPE_SW_UNCORRECTABLE		= 17,

	
	EC_ERR_TYPE_OTHER_HW_CORRECTABLE	= 32,

	
	EC_ERR_TYPE_OTHER_HW_UNCORRECTABLE	= 33,
};


enum ec_registers_group {
	
	EC_ERR_GROUP_FUNC_REG		= 0U,
	
	EC_ERR_GROUP_SCR_REG		= 1U,
};



#define EC_STATUS_FLAG_NO_ERROR		0x0001U

#define EC_STATUS_FLAG_LAST_ERROR	0x0002U

#define EC_STATUS_FLAG_LATENT_ERROR	0x0004U




#define EC_DESC_FLAG_RESOLVED		0x0001U

#define EC_DESC_FLAG_NO_ID		0x0002U



struct ec_err_fmon_desc {
	
	uint16_t desc_flags;
	
	uint16_t fmon_clk_id;
	
	uint32_t fmon_faults;
	
	int32_t fmon_access_error;
} BPMP_ABI_PACKED;


struct ec_err_vmon_desc {
	
	uint16_t desc_flags;
	
	uint16_t vmon_adc_id;
	
	uint32_t vmon_faults;
	
	int32_t vmon_access_error;
} BPMP_ABI_PACKED;


struct ec_err_reg_parity_desc {
	
	uint16_t desc_flags;
	
	uint16_t reg_id;
	
	uint16_t reg_group;
} BPMP_ABI_PACKED;


struct ec_err_sw_error_desc {
	
	uint16_t desc_flags;
	
	uint16_t err_source_id;
	
	uint32_t sw_error_data;
} BPMP_ABI_PACKED;


struct ec_err_simple_desc {
	
	uint16_t desc_flags;
	
	uint16_t err_source_id;
} BPMP_ABI_PACKED;


union ec_err_desc {
	struct ec_err_fmon_desc fmon_desc;
	struct ec_err_vmon_desc vmon_desc;
	struct ec_err_reg_parity_desc reg_parity_desc;
	struct ec_err_sw_error_desc sw_error_desc;
	struct ec_err_simple_desc simple_desc;
} BPMP_ABI_PACKED;

struct cmd_ec_status_get_request {
	
	uint32_t ec_hsm_id;
} BPMP_ABI_PACKED;


#define EC_ERR_STATUS_DESC_MAX_NUM	4U


struct cmd_ec_status_get_response {
	
	uint32_t ec_hsm_id;
	
	uint32_t ec_status_flags;
	
	uint32_t error_idx;
	
	uint32_t error_type;
	
	uint32_t error_desc_num;
	
	union ec_err_desc error_descs[EC_ERR_STATUS_DESC_MAX_NUM];
} BPMP_ABI_PACKED;


struct cmd_ec_status_ex_get_response {
	
	uint32_t ec_hsm_id;
	
	uint32_t ec_status_flags;
	
	uint32_t error_idx;
	
	uint32_t error_type;
	
	uint32_t error_counter;
	
	uint32_t error_uval;
	
	uint32_t reserved;
	
	uint32_t error_desc_num;
	
	union ec_err_desc error_descs[EC_ERR_STATUS_DESC_MAX_NUM];
} BPMP_ABI_PACKED;



struct mrq_ec_request {
	
	uint32_t cmd_id;

	union {
		struct cmd_ec_status_get_request ec_status_get;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;



struct mrq_ec_response {
	union {
		
		struct cmd_ec_status_get_response ec_status_get;
		
		struct cmd_ec_status_ex_get_response ec_status_ex_get;
	} BPMP_UNION_ANON;
} BPMP_ABI_PACKED;







#define BPMP_EPERM	1

#define BPMP_ENOENT	2

#define BPMP_ENOHANDLER	3

#define BPMP_EIO	5

#define BPMP_EBADCMD	6

#define BPMP_EAGAIN	11

#define BPMP_ENOMEM	12

#define BPMP_EACCES	13

#define BPMP_EFAULT	14

#define BPMP_EBUSY	16

#define BPMP_ENODEV	19

#define BPMP_EISDIR	21

#define BPMP_EINVAL	22

#define BPMP_ETIMEDOUT  23

#define BPMP_ERANGE	34

#define BPMP_ENOSYS	38

#define BPMP_EBADSLT	57

#define BPMP_ENOTSUP	134

#define BPMP_ENXIO	140



#if defined(BPMP_ABI_CHECKS)
#include "bpmp_abi_checks.h"
#endif

#endif
