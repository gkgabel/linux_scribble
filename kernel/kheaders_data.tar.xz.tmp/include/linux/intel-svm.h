/* SPDX-License-Identifier: GPL-2.0-only */


#ifndef __INTEL_SVM_H__
#define __INTEL_SVM_H__


#define PRQ_ORDER	4
#define PRQ_RING_MASK	((0x1000 << PRQ_ORDER) - 0x20)
#define PRQ_DEPTH	((0x1000 << PRQ_ORDER) >> 5)


#define SVM_FLAG_SUPERVISOR_MODE	BIT(0)

#endif 
