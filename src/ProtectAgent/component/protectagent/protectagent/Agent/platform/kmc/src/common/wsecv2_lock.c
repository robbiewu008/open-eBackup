/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: common lock function implementation
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#include "wsecv2_lock.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_callbacks.h"

static WsecHandle g_lockEx[WSEC_LOCK_NUM] = {0};
static WsecHandle g_kmcProcLock[WSEC_PROC_LOCK_NUM] = {0};
static WsecLockRegStatus g_cbbSysEx = {WSEC_LOCK_UNGEN};   /* Check whether the lock function is registered. */

/* Obtains the lock status. */
WsecUint32 GetLockStatus(void)
{
    return g_cbbSysEx.state;
}

/* Initialize the lock (process lock + thread lock). */
unsigned long WsecInitializeLock(void)
{
    int i; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    int j;
    if (g_cbbSysEx.state == WSEC_LOCK_GEN) {
        return WSEC_SUCCESS;
    }

    for (i = 0; i < (int)WSEC_NUM_OF(g_lockEx); i++) {
        WsecDestroyThreadLock(&(g_lockEx[i])); /* Ensure that the lock is not created. */
        /* If the operation fails, the created lock is destroyed. */
        if (WsecCreateThreadLock(&g_lockEx[i]) != WSEC_SUCCESS) {
            for (j = 0; j < i; j++) {
                WsecDestroyThreadLock(&(g_lockEx[j]));
            }
            return WSEC_FAILURE;
        }
    }

    for (i = 0; i < (int)WSEC_NUM_OF(g_kmcProcLock); i++) {
        WsecDestroyProcLock(&(g_kmcProcLock[i])); /* Ensure that the process lock is not created. */
        /* If the operation fails, the thread and process locks are destroyed. */
        if (WsecCreateProcLock(&g_kmcProcLock[i]) != WSEC_SUCCESS) {
            for (j = 0; j < (int)WSEC_NUM_OF(g_lockEx); j++) {
                WsecDestroyThreadLock(&(g_lockEx[j]));
            }
            for (j = 0; j < i; j++) {
                WsecDestroyProcLock(&(g_kmcProcLock[j]));
            }
            return WSEC_FAILURE;
        }
    }

    g_cbbSysEx.state = WSEC_LOCK_GEN;
    return WSEC_SUCCESS;
}

/* Lock destruction */
WsecVoid WsecFinalizeLock(void)
{
    int i; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    if (g_cbbSysEx.state == WSEC_LOCK_UNGEN) {
        return;
    }
    for (i = 0; i < (int)WSEC_NUM_OF(g_lockEx); i++) {
        WsecDestroyThreadLock(&(g_lockEx[i]));
    }
    for (i = 0; i < (int)WSEC_NUM_OF(g_kmcProcLock); i++) {
        WsecDestroyProcLock(&(g_kmcProcLock[i]));
    }
    g_cbbSysEx.state = WSEC_LOCK_UNGEN;
}

/* Adds a thread lock to a resource by ID. */
WsecVoid WsecThreadLockById(WsecUint32 lockId)
{
    WsecThreadLock(g_lockEx[lockId]);
}

/* Unlocks a resource by ID. */
WsecVoid WsecThreadUnlockById(WsecUint32 lockId)
{
    WsecThreadUnlock(g_lockEx[lockId]);
}

/* Adds a process lock to a resource by ID. */
WsecVoid WsecProcLockById(WsecUint32 lockId)
{
    WsecProcLock(g_kmcProcLock[lockId]);
}

/* Releases a process by ID. */
WsecVoid WsecProcUnlockById(WsecUint32 lockId)
{
    WsecProcUnlock(g_kmcProcLock[lockId]);
}
