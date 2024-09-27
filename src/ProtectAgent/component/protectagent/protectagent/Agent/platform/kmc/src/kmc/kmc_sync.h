/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC - Key Management Component - Sync MK
 * Author: yangdingfu
 * Create: 20-11-03
 * Notes: This File split from kmcv2_ksm.c since the original file is near 2000 lines.
 */

#ifndef KMC_SRC_KMC_KMC_SYNC_H
#define KMC_SRC_KMC_KMC_SYNC_H


#include "wsecv2_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* MASTER: indicates that the AGENT sends all shared master keys. */
unsigned long MemMasterSendMkByDomain(WsecUint32 domainId, WsecVoid *param, CallbackSendSyncData sendSyncData);

/* The AGENT receives all shared master keys from the master. */
unsigned long MemAgentRecvMkByDomain(WsecUint32 recvMode,
    WsecUint32 *outDomainId, WsecVoid *param, CallbackRecvSyncData recvSyncData);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* KMC_SRC_KMC_KMC_SYNC_H */
