/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC internal interfaces are not open to external systems.
 * Author: x00102361
 * Create: 2014-06-16
 */

#ifndef KMC_SRC_COMMON_WSECV2_LOCK_H
#define KMC_SRC_COMMON_WSECV2_LOCK_H

#include "wsecv2_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOCK4KEYSTORE = 0, /* Keystore data and corresponding files in the memory */
    LOCK4KMC_CFG  = 1, /* KMC Memory Configuration */
    LOCK4KMC_RAND = 2, /* Random number generation lock */
    WSEC_LOCK_NUM = 3
} WsecThreadLockFor;

typedef enum {
    PROCLOCK4KEYSTORE  = 0,
    WSEC_PROC_LOCK_NUM = 1
} WsecProclockFor;

typedef enum {
    WSEC_LOCK_UNGEN = 0, /* The lock is not created. */
    WSEC_LOCK_GEN        /* A lock has been created. */
} WsecLockGen;

/* 4. Other data types and structures */
typedef struct TagWsecLockRegStatus {
    WsecUint32 state;
} WsecLockRegStatus;

/* Initialize the lock (process lock + thread lock). */
unsigned long WsecInitializeLock(void);

/* Lock destruction */
WsecVoid WsecFinalizeLock(void);

/* Adds a thread lock to a resource by ID. */
WsecVoid WsecThreadLockById(WsecUint32 lockId);

/* Unlocks a resource by ID. */
WsecVoid WsecThreadUnlockById(WsecUint32 lockId);

/* Adds a process lock to a resource by ID. */
WsecVoid WsecProcLockById(WsecUint32 lockId);

/* Releases a process by ID. */
WsecVoid WsecProcUnlockById(WsecUint32 lockId);

/* Obtains the lock status. */
WsecUint32 GetLockStatus(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* KMC_SRC_COMMON_WSECV2_LOCK_H */
