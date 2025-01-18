#undef TRACE_SYSTEM
#define TRACE_SYSTEM migration

#if !defined(_TRACE_MIGRATION_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_MIGRATION_H

#include <linux/tracepoint.h>

// Define the tracepoint
TRACE_EVENT(migrate_event,

    TP_PROTO(u64 arg, const char *msg),

    TP_ARGS(arg, msg),

    TP_STRUCT__entry(
        __field(u64, arg)
        __string(msg, msg)
    ),

    TP_fast_assign(
        __entry->arg = arg;
        __assign_str(msg, msg);
    ),

    TP_printk("arg=%llu msg=%s", __entry->arg, __get_str(msg))
);

// Define the tracepoint
TRACE_EVENT(iommu_map_event,

    TP_PROTO(u64 arg1,u64 arg2,u64 arg3, const char *msg),

    TP_ARGS(arg1, arg2, arg3, msg),

    TP_STRUCT__entry(
        __field(u64, arg1)
        __field(u64, arg2)
        __field(u64, arg3)
        __string(msg, msg)
    ),

    TP_fast_assign(
        __entry->arg1 = arg1;
        __entry->arg2 = arg2;
        __entry->arg3 = arg3;
        __assign_str(msg, msg);
    ),

    TP_printk("arg1=%llu arg2=%llu arg3=%llu msg=%s", __entry->arg1, __entry->arg2, __entry->arg3, __get_str(msg))
);

// Define the tracepoint
TRACE_EVENT(iommu_unmap_event,

    TP_PROTO(u64 arg1,u64 arg2, const char *msg),

    TP_ARGS(arg1, arg2, msg),

    TP_STRUCT__entry(
        __field(u64, arg1)
        __field(u64, arg2)
        __string(msg, msg)
    ),

    TP_fast_assign(
        __entry->arg1 = arg1;
        __entry->arg2 = arg2;
        __assign_str(msg, msg);
    ),

    TP_printk("arg1=%llu arg2=%llu msg=%s", __entry->arg1, __entry->arg2, __get_str(msg))
);
//tracepoints look like this
//trace_iommu_unmap_event(iov_pfn, start_pfn, last_pfn, "iommu unmap event");
//trace_iommu_clear_pte(start_pfn, last_pfn, dma_pte_addr(pte)>>VTD_PAGE_SHIFT, "iommu clear pte event");

// Define the tracepoint
TRACE_EVENT(iommu_clear_pte,

    TP_PROTO(u64 arg1,u64 arg2,u64 arg3, const char *msg),

    TP_ARGS(arg1, arg2, arg3, msg),

    TP_STRUCT__entry(
        __field(u64, arg1)
        __field(u64, arg2)
        __field(u64, arg3)
        __string(msg, msg)
    ),

    TP_fast_assign(
        __entry->arg1 = arg1;
        __entry->arg2 = arg2;
        __entry->arg3 = arg3;
        __assign_str(msg, msg);
    ),

    TP_printk("arg1=%llu arg2=%llu arg3=%llu msg=%s", __entry->arg1, __entry->arg2, __entry->arg3, __get_str(msg))
);
#endif /* _TRACE_MIGRATION_H */

#include <trace/define_trace.h>
