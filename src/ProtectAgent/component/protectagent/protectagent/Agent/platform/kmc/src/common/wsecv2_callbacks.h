/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: KMC internal interfaces are not open to external systems.
 * Author: z00316590
 * Create: 2018-11-08
 */

#ifndef KMC_SRC_COMMON_WSECV2_CALLBACKS_H
#define KMC_SRC_COMMON_WSECV2_CALLBACKS_H

#include "wsecv2_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Macro */
/* Notification to the app */
#define WSEC_NOTIFY(notifyCode, data, size) WsecNotify((WsecUint32)(notifyCode), data, (size_t)(size))

/* Hand over the execution rights of the CPU. */
#define WSEC_DO_EVENTS WsecDoEvents()

/* Provide Random Number Interface */
WsecBool WsecIsRngSupplied(void);

/* Sets callback functions. */
/* (Optional) Setting the Memory Operation Callback Function */
unsigned long WsecSetMemCallbacks(const WsecMemCallbacks *memCallbacks);

/* (Mandatory) Set the file operation callback function. */
unsigned long WsecSetFileCallbacks(const WsecFileCallbacks *fileCallbacks);

/* (Mandatory) Set the thread lock callback function. */
unsigned long WsecSetThreadLockCallbacks(const WsecLockCallbacks *lockCallbacks);

/* (Mandatory) Set the process lock callback function. */
unsigned long WsecSetProcLockCallbacks(const WsecProcLockCallbacks *procLockCallbacks);

/* (Mandatory) Set the callback function for logs, notifications, and processing time. */
unsigned long WsecSetBasicRelyCallbacks(const WsecBasicRelyCallbacks *basicRelyCallbacks);

/*
 * Set the callback function for random number generation
 * and entropy obtaining (either random number obtaining or entropy obtaining)
 */
unsigned long WsecSetRngCallbacks(const WsecRngCallbacks *rngCallbacks);

/* (Mandatory) Set the callback function for obtaining the UTC time. */
unsigned long WsecSetTimeCallbacks(const WsecTimeCallbacks *timeCallbacks);

/* (Optional) Setting Hardware Access Callback */
unsigned long WsecSetHardwareCallbacks(const WsecHardwareCallbacks *hardwareCallbacks);

/* Encapsulate various callbacks. */
/* Memory */
WsecVoid *WsecMalloc(size_t size);

WsecVoid WsecFree(WsecVoid *memBuff);

int WsecMemCmp(const WsecVoid *buffA, const WsecVoid *buffB, size_t count);

/* File */
WsecHandle WsecFopen(const char *filePathName, const KmcFileOpenMode mode);

int WsecFclose(WsecHandle stream);

WsecBool WsecFread(WsecVoid *buffer, size_t count, WsecHandle stream);

WsecBool WsecFwrite(const WsecVoid *buffer, size_t count, WsecHandle stream);

int WsecFflush(WsecHandle stream);

int WsecFremove(const char *path);

long WsecFtell(WsecHandle stream);

long WsecFseek(WsecHandle stream, long offset, KmcFileSeekPos origin);

int WsecFeof(WsecHandle stream, WsecBool *endOfFile);

int WsecFerrno(WsecHandle stream);

unsigned long WsecFileCheck(const char *filePathName, WsecBool *fileExist);

/* Thread lock class. */
unsigned long WsecCreateThreadLock(WsecHandle *mutexObject);

WsecVoid WsecDestroyThreadLock(WsecHandle *mutexObject);

WsecVoid WsecThreadLock(WsecHandle mutexObject);

WsecVoid WsecThreadUnlock(WsecHandle mutexObject);

/* Process lock class. */
unsigned long WsecCreateProcLock(WsecHandle *mutexObject);

WsecVoid WsecDestroyProcLock(WsecHandle *mutexObject);

WsecVoid WsecProcLock(WsecHandle mutexObject);

WsecVoid WsecProcUnlock(WsecHandle mutexObject);

/* Encapsulates basic dependency class callback. */
WsecVoid WsecWriteLog(int level,
    const char *moduleName,
    const char *filePathName,
    int lineNum,
    const char *logString);

WsecVoid WsecDoEvents(void);

WsecVoid WsecNotify(WsecUint32 notifyCode, const WsecVoid *data, size_t dataSize);

/* Random number type 1: A random number generator is invoked to generate random numbers. */
WsecBool WsecGetRandomNumber(unsigned char *buff, size_t buffLen);

/*
 * Random number type 2: calling the external entropy value callback function
 * to obtain the entropy value and destroying the entropy value buffer
 */
WsecBool WsecGetEntropy(unsigned char **entropyBuff, size_t buffLen);

WsecVoid WsecCleanupEntropy(unsigned char *entropyBuff, size_t buffLen);

/* Time class (The gmtime cannot be reentered and can be reentered through callback, for example, linux gmtime_r.) */
WsecBool WsecGmTime(const time_t *curTime, struct tm *curTm);

/* Obtaining Additional Encryption Parameters */
unsigned long WsecHwGetEncExtraData(const unsigned char **extraData, unsigned int *extraLen);

/* Obtaining Additional Decryption Parameters */
unsigned long WsecHwGetDecExtraData(const unsigned char **extraData, unsigned int *extraLen);

/* Obtains the length of persistent data. */
unsigned long WsecHwGetPersistentDataLen(unsigned int *len);

/* Initializing the Hardware Key Manager */
unsigned long WsecHwInitKeyMgr(const void *passthroughData, unsigned int passthroughDataLen);

/* Create a new key on the hardware. */
unsigned long WsecHwNewRootKey(unsigned char *persistentData, unsigned int *persistentDataLen, WsecHandle *handle);

/* Loading Hardware Keys */
unsigned long WsecHwLoadRootkey(const unsigned char *persistentData, unsigned int persistentDataLen,
    WsecHandle *handle);

/* Obtains the ciphertext length. */
unsigned long WsecHwGetCipherLen(unsigned int plaintextLen, unsigned int *ciphertextLen);

/* Encryption (software-layer root key or master key) */
unsigned long WsecHwEncData(WsecHandle handle, const unsigned char *extraData, unsigned int extraLen,
    const WsecPlainCipherBuffs *buffs);

/* Decryption (software-layer root key or master key) */
unsigned long WsecHwDecData(WsecHandle handle, const unsigned char *extraData, unsigned int extraLen,
    const WsecPlainCipherBuffs *buffs);

/* Unload hardware root key */
unsigned long WsecHwUnloadKey(WsecHandle handle);

/* Removing the Hardware Root Key */
unsigned long WsecHwRemoveKey(WsecHandle handle);

/* Deinitializes the hardware key manager. */
unsigned long WsecHwUninitKeyMgr(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* KMC_SRC_COMMON_WSECV2_CALLBACKS_H */
