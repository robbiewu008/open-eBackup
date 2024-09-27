/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: The APP can use the CBB interface programming by including only the header file.
 * If the CBB needs to be tailored, go to
 * The wsecv2_config.h file enables or disables the sub CBB compilation macro based on comments.
 * Author: Luan Shipeng l00171031
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#ifndef KMC_INCLUDE_WSECV2_CBB_H
#define KMC_INCLUDE_WSECV2_CBB_H

#include "wsecv2_config.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_itf.h"
#include "kmcv2_itf.h"

#ifdef WSEC_COMPILE_SDP
#include "sdpv2_itf.h"
#include "sdpv1_itf.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* KMC_INCLUDE_WSECV2_CBB_H */
