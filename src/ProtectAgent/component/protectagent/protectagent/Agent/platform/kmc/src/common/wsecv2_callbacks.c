/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: Callback function encapsulation
 * Author: z00316590
 * Create: 2018-11-08
 */

#include "wsecv2_callbacks.h"
#include "wsecv2_errorcode.h"

static int DefaultMemCmp(const WsecVoid *buffA, const WsecVoid *buffB, size_t count);
static WsecVoid *DefaultMalloc(size_t size);
static WsecVoid DefaultFree(WsecVoid *memBuff);

/* Registering a Function */
static WsecCallbacks g_regCallbacks = {
    /* WsecMemCallbacks */
    { DefaultMalloc, DefaultFree, DefaultMemCmp },

    /* WsecFileCallbacks */
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },

    /* WsecLockCallbacks */
    { NULL, NULL, NULL, NULL },

    /* WsecProcLockCallbacks */
    { NULL, NULL, NULL, NULL },

    /* WsecBasicRelyCallbacks */
    { NULL, NULL, NULL },

    /* WsecRngCallbacks */
    { NULL, NULL, NULL },

    /* WsecTimeCallbacks */
    { NULL },

    /* WsecHardwareCallbacks */
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

/* Default memory allocation function */
static WsecVoid *DefaultMalloc(size_t size)
{
    WsecVoid *ptr = NULL;
    if (size == 0) {
        return ptr;
    }
    ptr = malloc(size);
    if (ptr == NULL) {
        return NULL;
    }
    return ptr;
}

/* Default memory release function */
static WsecVoid DefaultFree(WsecVoid *memBuff)
{
    free(memBuff);
}

/* Default memory comparison function */
static int DefaultMemCmp(const WsecVoid *buffA, const WsecVoid *buffB, size_t count)
{
    return memcmp(buffA, buffB, count);
}

/* Count the number of elements whose values are 0 in the buffer. */
static size_t GetZeroItemCount(const WsecVoid *data, size_t size, size_t itemSize)
{
    size_t count = 0;
    size_t i;
    unsigned char buffZeros[16] = {0}; /* A single item supports a maximum of 16 bytes. */
    const unsigned char *oneItemBuff = NULL;

    WSEC_ASSERT((size % itemSize) == 0);
    WSEC_ASSERT(itemSize <= sizeof(buffZeros));
    oneItemBuff = (const unsigned char *)data;
    for (i = 0; i < size; i += itemSize) {
        if (WsecMemCmp(&oneItemBuff[i], buffZeros, itemSize) == 0) {
            count++;
        }
    }

    return count;
}

/* Provide Random Number Interface */
WsecBool WsecIsRngSupplied(void)
{
    return (g_regCallbacks.rngCallbacks.getRandomNum != NULL);
}

/* (Optional) Set the memory operation callback function. */
unsigned long WsecSetMemCallbacks(const WsecMemCallbacks *memCallbacks)
{
    size_t nullCallbackNum;

    WSEC_ASSERT(memCallbacks != NULL);
    nullCallbackNum = GetZeroItemCount(memCallbacks, sizeof(WsecMemCallbacks), sizeof(WsecVoid *));
    /* The options are provided or not provided. Otherwise, the parameter is incorrect. */
    if (nullCallbackNum > 0 && nullCallbackNum != (sizeof(WsecMemCallbacks) / sizeof(WsecVoid *))) {
        return WSEC_ERR_INVALID_ARG;
    }
    if (nullCallbackNum == 0) {
        g_regCallbacks.memCallbacks.memAlloc = memCallbacks->memAlloc;
        g_regCallbacks.memCallbacks.memCmp = memCallbacks->memCmp;
        g_regCallbacks.memCallbacks.memFree = memCallbacks->memFree;
    }
    return WSEC_SUCCESS;
}

/* File operation callback function (mandatory) */
unsigned long WsecSetFileCallbacks(const WsecFileCallbacks *fileCallbacks)
{
    size_t nullCallbackNum;

    WSEC_ASSERT(fileCallbacks != NULL);
    nullCallbackNum = GetZeroItemCount(fileCallbacks, sizeof(WsecFileCallbacks), sizeof(WsecVoid *));
    /* All parameters must be provided. Otherwise, the parameter is incorrect. */
    if (nullCallbackNum > 0) {
        return WSEC_ERR_INVALID_ARG;
    }
    g_regCallbacks.fileCallbacks.fileOpen = fileCallbacks->fileOpen;
    g_regCallbacks.fileCallbacks.fileClose = fileCallbacks->fileClose;
    g_regCallbacks.fileCallbacks.fileRead = fileCallbacks->fileRead;
    g_regCallbacks.fileCallbacks.fileWrite = fileCallbacks->fileWrite;
    g_regCallbacks.fileCallbacks.fileFlush = fileCallbacks->fileFlush;
    g_regCallbacks.fileCallbacks.fileSeek = fileCallbacks->fileSeek;
    g_regCallbacks.fileCallbacks.fileTell = fileCallbacks->fileTell;
    g_regCallbacks.fileCallbacks.fileEof = fileCallbacks->fileEof;
    g_regCallbacks.fileCallbacks.fileErrno = fileCallbacks->fileErrno;
    g_regCallbacks.fileCallbacks.fileExist = fileCallbacks->fileExist;
    g_regCallbacks.fileCallbacks.fileRemove = fileCallbacks->fileRemove;
    return WSEC_SUCCESS;
}

/* (Mandatory) Set the thread lock callback function. */
unsigned long WsecSetThreadLockCallbacks(const WsecLockCallbacks *lockCallbacks)
{
    size_t nullCallbackNum;

    WSEC_ASSERT(lockCallbacks != NULL);
    nullCallbackNum = GetZeroItemCount(lockCallbacks, sizeof(WsecLockCallbacks), sizeof(WsecVoid *));
    /* All parameters must be provided. Otherwise, the parameter is incorrect. */
    if (nullCallbackNum > 0) {
        return WSEC_ERR_INVALID_ARG;
    }
    g_regCallbacks.lockCallbacks.createLock = lockCallbacks->createLock;
    g_regCallbacks.lockCallbacks.lock = lockCallbacks->lock;
    g_regCallbacks.lockCallbacks.unlock = lockCallbacks->unlock;
    g_regCallbacks.lockCallbacks.destroyLock = lockCallbacks->destroyLock;
    return WSEC_SUCCESS;
}

/* (Mandatory) Set the process lock callback function. */
unsigned long WsecSetProcLockCallbacks(const WsecProcLockCallbacks *procLockCallbacks)
{
    size_t nullCallbackNum;

    WSEC_ASSERT(procLockCallbacks != NULL);
    nullCallbackNum = GetZeroItemCount(procLockCallbacks, sizeof(WsecProcLockCallbacks), sizeof(WsecVoid *));
    /* All parameters must be provided. Otherwise, the parameter is incorrect. */
    if (nullCallbackNum > 0) {
        return WSEC_ERR_INVALID_ARG;
    }
    g_regCallbacks.procLockCallbacks.createProcLock = procLockCallbacks->createProcLock;
    g_regCallbacks.procLockCallbacks.procLock = procLockCallbacks->procLock;
    g_regCallbacks.procLockCallbacks.procUnlock = procLockCallbacks->procUnlock;
    g_regCallbacks.procLockCallbacks.destroyProcLock = procLockCallbacks->destroyProcLock;
    return WSEC_SUCCESS;
}

/* (Mandatory) Set the callback function for logs, notifications, and processing time. */
unsigned long WsecSetBasicRelyCallbacks(const WsecBasicRelyCallbacks *basicRelyCallbacks)
{
    size_t nullCallbackNum;

    WSEC_ASSERT(basicRelyCallbacks != NULL);
    nullCallbackNum = GetZeroItemCount(basicRelyCallbacks, sizeof(WsecBasicRelyCallbacks), sizeof(WsecVoid *));
    /* All parameters must be provided. Otherwise, the parameter is incorrect. */
    if (nullCallbackNum > 0) {
        return WSEC_ERR_INVALID_ARG;
    }
    g_regCallbacks.basicRelyCallbacks.writeLog = basicRelyCallbacks->writeLog;
    g_regCallbacks.basicRelyCallbacks.notify = basicRelyCallbacks->notify;
    g_regCallbacks.basicRelyCallbacks.doEvents = basicRelyCallbacks->doEvents;
    return WSEC_SUCCESS;
}

/*
 * Sets the callback function for random number generation and
 * entropy obtaining (either random number obtaining or entropy obtaining).
 */
unsigned long WsecSetRngCallbacks(const WsecRngCallbacks *rngCallbacks)
{
    WSEC_ASSERT(rngCallbacks != NULL);
    /* The getRandomNum, getEntropy, and cleanupEntropy must be provided. Otherwise, the parameter is incorrect. */
    if (!(rngCallbacks->getRandomNum != NULL ||
        (rngCallbacks->getEntropy != NULL && rngCallbacks->cleanupEntropy != NULL))) {
        return WSEC_ERR_INVALID_ARG;
    }
    g_regCallbacks.rngCallbacks.getRandomNum = rngCallbacks->getRandomNum;
    g_regCallbacks.rngCallbacks.getEntropy = rngCallbacks->getEntropy;
    g_regCallbacks.rngCallbacks.cleanupEntropy = rngCallbacks->cleanupEntropy;
    return WSEC_SUCCESS;
}

/* (Mandatory) Set the callback function for obtaining the UTC time. */
unsigned long WsecSetTimeCallbacks(const WsecTimeCallbacks *timeCallbacks)
{
    WSEC_ASSERT(timeCallbacks != NULL);
    /* All parameters must be provided. Otherwise, the parameter is incorrect. */
    if (timeCallbacks->gmTimeSafe == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }
    g_regCallbacks.timeCallbacks.gmTimeSafe = timeCallbacks->gmTimeSafe;
    return WSEC_SUCCESS;
}

/* (Optional) Setting the Hardware Callback Function */
unsigned long WsecSetHardwareCallbacks(const WsecHardwareCallbacks *hardwareCallbacks)
{
    size_t nullCallbackNum;
    WSEC_ASSERT(hardwareCallbacks != NULL);
    nullCallbackNum = GetZeroItemCount(hardwareCallbacks, sizeof(WsecHardwareCallbacks), sizeof(WsecVoid *));
    if (nullCallbackNum != 0 && nullCallbackNum != sizeof(WsecHardwareCallbacks) / sizeof(WsecVoid *)) {
        return WSEC_ERR_INVALID_ARG;
    }
    g_regCallbacks.hardwareCallbacks.hwGetEncExtraData = hardwareCallbacks->hwGetEncExtraData;
    g_regCallbacks.hardwareCallbacks.hwGetDecExtraData = hardwareCallbacks->hwGetDecExtraData;
    g_regCallbacks.hardwareCallbacks.hwGetPersistentDataLen = hardwareCallbacks->hwGetPersistentDataLen;
    g_regCallbacks.hardwareCallbacks.hwInitKeyMgr = hardwareCallbacks->hwInitKeyMgr;
    g_regCallbacks.hardwareCallbacks.hwNewRootKey = hardwareCallbacks->hwNewRootKey;
    g_regCallbacks.hardwareCallbacks.hwLoadRootkey = hardwareCallbacks->hwLoadRootkey;
    g_regCallbacks.hardwareCallbacks.hwGetCipherLen = hardwareCallbacks->hwGetCipherLen;
    g_regCallbacks.hardwareCallbacks.hwEncData = hardwareCallbacks->hwEncData;
    g_regCallbacks.hardwareCallbacks.hwDecData = hardwareCallbacks->hwDecData;
    g_regCallbacks.hardwareCallbacks.hwUnloadKey = hardwareCallbacks->hwUnloadKey;
    g_regCallbacks.hardwareCallbacks.hwRemoveKey = hardwareCallbacks->hwRemoveKey;
    g_regCallbacks.hardwareCallbacks.hwUninitKeyMgr = hardwareCallbacks->hwUninitKeyMgr;
    return WSEC_SUCCESS;
}

/* Callback function encapsulation: memory allocation */
WsecVoid *WsecMalloc(size_t size)
{
    return g_regCallbacks.memCallbacks.memAlloc(size);
}

/* Callback function encapsulation: memory release */
WsecVoid WsecFree(WsecVoid *memBuff)
{
    if (memBuff == NULL) {
        return;
    }
    g_regCallbacks.memCallbacks.memFree(memBuff);
}

/* Callback function encapsulation: memory comparison */
int WsecMemCmp(const WsecVoid *buffA, const WsecVoid *buffB, size_t count)
{
    return g_regCallbacks.memCallbacks.memCmp(buffA, buffB, count);
}

/* Callback function encapsulation: file opening */
WsecHandle WsecFopen(const char *filePathName, const KmcFileOpenMode mode)
{
    if (g_regCallbacks.fileCallbacks.fileOpen == NULL) {
        return NULL;
    }
    return g_regCallbacks.fileCallbacks.fileOpen(filePathName, mode);
}

/* Callback function encapsulation: closing a file */
int WsecFclose(WsecHandle stream)
{
    if (g_regCallbacks.fileCallbacks.fileClose == NULL) {
        return -1;
    }
    return g_regCallbacks.fileCallbacks.fileClose(stream);
}

/* Callback function encapsulation: file reading */
WsecBool WsecFread(WsecVoid *buffer, size_t count, WsecHandle stream)
{
    if (g_regCallbacks.fileCallbacks.fileRead == NULL) {
        return WSEC_FALSE;
    }
    return g_regCallbacks.fileCallbacks.fileRead(buffer, count, stream);
}

/* Callback function encapsulation: file writing */
WsecBool WsecFwrite(const WsecVoid *buffer, size_t count, WsecHandle stream)
{
    if (g_regCallbacks.fileCallbacks.fileWrite == NULL) {
        return WSEC_FALSE;
    }
    return g_regCallbacks.fileCallbacks.fileWrite(buffer, count, stream);
}

/* Callback function encapsulation: file flush */
int WsecFflush(WsecHandle stream)
{
    if (g_regCallbacks.fileCallbacks.fileFlush == NULL) {
        return -1;
    }
    /*
     * The synchronization is performed only when a handle exists.
     * Empty handles do not need to be synchronized. (The synchronization is considered successful.)
     */
    if (stream == NULL) {
        return 0;
    }
    return g_regCallbacks.fileCallbacks.fileFlush(stream);
}

/* Callback function encapsulation: file deletion */
int WsecFremove(const char *path)
{
    if (g_regCallbacks.fileCallbacks.fileRemove == NULL) {
        return -1;
    }
    return g_regCallbacks.fileCallbacks.fileRemove(path);
}

/* Callback function encapsulation: file offset obtaining */
long WsecFtell(WsecHandle stream)
{
    if (g_regCallbacks.fileCallbacks.fileTell == NULL) {
        return -1;
    }
    return g_regCallbacks.fileCallbacks.fileTell(stream);
}

/* Callback function encapsulation: file offset setting */
long WsecFseek(WsecHandle stream, long offset, KmcFileSeekPos origin)
{
    if (g_regCallbacks.fileCallbacks.fileSeek == NULL) {
        return -1;
    }
    return g_regCallbacks.fileCallbacks.fileSeek(stream, offset, origin);
}

/* Callback function encapsulation: file end judgment */
int WsecFeof(WsecHandle stream, WsecBool *endOfFile)
{
    if (g_regCallbacks.fileCallbacks.fileEof == NULL || endOfFile == NULL) {
        return -1;
    }
    return g_regCallbacks.fileCallbacks.fileEof(stream, endOfFile);
}

/* Callback function encapsulation: file operation error code */
int WsecFerrno(WsecHandle stream)
{
    if (g_regCallbacks.fileCallbacks.fileErrno == NULL) {
        return -1;
    }
    return g_regCallbacks.fileCallbacks.fileErrno(stream);
}

/* Callback function encapsulation: file existence check */
unsigned long WsecFileCheck(const char *filePathName, WsecBool *fileExist)
{
    WSEC_ASSERT(filePathName != NULL);
    WSEC_ASSERT(fileExist != NULL);
    if (g_regCallbacks.fileCallbacks.fileExist == NULL) {
        return WSEC_FAILURE;
    }
    *fileExist = g_regCallbacks.fileCallbacks.fileExist(filePathName);
    return WSEC_SUCCESS;
}

/* Callback function encapsulation: creating a thread lock */
unsigned long WsecCreateThreadLock(WsecHandle *mutexObject)
{
    if ((g_regCallbacks.lockCallbacks.createLock == NULL) || (mutexObject == NULL)) {
        return WSEC_FAILURE;
    }
    return g_regCallbacks.lockCallbacks.createLock(mutexObject) ? WSEC_SUCCESS : WSEC_FAILURE;
}

/* Callback function encapsulation: destroys the thread lock. */
WsecVoid WsecDestroyThreadLock(WsecHandle *mutexObject)
{
    if (mutexObject != NULL && (*mutexObject) != NULL && g_regCallbacks.lockCallbacks.destroyLock != NULL) {
        g_regCallbacks.lockCallbacks.destroyLock(*mutexObject);
        *mutexObject = NULL;
    }
}

/* Callback function encapsulation: locking the thread lock */
WsecVoid WsecThreadLock(WsecHandle mutexObject)
{
    if (mutexObject != NULL && g_regCallbacks.lockCallbacks.lock != NULL) {
        g_regCallbacks.lockCallbacks.lock(mutexObject);
    }
}

/* Callback function encapsulation: unlocking the thread lock */
WsecVoid WsecThreadUnlock(WsecHandle mutexObject)
{
    if (mutexObject != NULL && g_regCallbacks.lockCallbacks.unlock != NULL) {
        g_regCallbacks.lockCallbacks.unlock(mutexObject);
    }
}

/* Callback function encapsulation: creating a process lock */
unsigned long WsecCreateProcLock(WsecHandle *mutexObject)
{
    if (g_regCallbacks.procLockCallbacks.createProcLock == NULL || mutexObject == NULL) {
        return WSEC_FAILURE;
    }
    return g_regCallbacks.procLockCallbacks.createProcLock(mutexObject) ? WSEC_SUCCESS : WSEC_FAILURE;
}

/* Callback function encapsulation: destroys the process lock. */
WsecVoid WsecDestroyProcLock(WsecHandle *mutexObject)
{
    if (mutexObject != NULL && (*mutexObject) != NULL && g_regCallbacks.procLockCallbacks.destroyProcLock != NULL) {
        g_regCallbacks.procLockCallbacks.destroyProcLock(*mutexObject);
        *mutexObject = NULL;
    }
}

/* Callback function encapsulation: locking the process */
WsecVoid WsecProcLock(WsecHandle mutexObject)
{
    if (mutexObject != NULL && g_regCallbacks.procLockCallbacks.procLock != NULL) {
        g_regCallbacks.procLockCallbacks.procLock(mutexObject);
    }
}

/* Callback function encapsulation: unlocking the process lock */
WsecVoid WsecProcUnlock(WsecHandle mutexObject)
{
    if (mutexObject != NULL && g_regCallbacks.procLockCallbacks.procUnlock != NULL) {
        g_regCallbacks.procLockCallbacks.procUnlock(mutexObject);
    }
}

/* Callback function encapsulation: log function */
WsecVoid WsecWriteLog(int level,
    const char *moduleName,
    const char *filePathName,
    int lineNum,
    const char *logString)
{
    if (g_regCallbacks.basicRelyCallbacks.writeLog == NULL) {
        return;
    }
    g_regCallbacks.basicRelyCallbacks.writeLog(level, moduleName, filePathName, lineNum, logString);
}

/* Callback function encapsulation: event processing function */
WsecVoid WsecDoEvents(void)
{
    if (g_regCallbacks.basicRelyCallbacks.doEvents == NULL) {
        return;
    }
    g_regCallbacks.basicRelyCallbacks.doEvents();
}

/* Callback function encapsulation: message notification function */
WsecVoid WsecNotify(WsecUint32 notifyCode, const WsecVoid *data, size_t dataSize)
{
    if (g_regCallbacks.basicRelyCallbacks.notify == NULL) {
        return;
    }
    g_regCallbacks.basicRelyCallbacks.notify(notifyCode, data, dataSize);
}

/* Callback function encapsulation: The random number generator is invoked to generate random numbers. */
WsecBool WsecGetRandomNumber(unsigned char *buff, size_t buffLen)
{
    if (g_regCallbacks.rngCallbacks.getRandomNum == NULL) {
        return WSEC_FALSE;
    }
    return g_regCallbacks.rngCallbacks.getRandomNum(buff, buffLen);
}

/*
 * Callback function encapsulation: Call the external entropy value callback function to obtain
 * the entropy value (the buffer is applied for in the callback function and the entropy value is filled).
 */
WsecBool WsecGetEntropy(unsigned char **entropyBuff, size_t buffLen)
{
    if (g_regCallbacks.rngCallbacks.getEntropy == NULL) {
        return WSEC_FALSE;
    }
    return g_regCallbacks.rngCallbacks.getEntropy(entropyBuff, buffLen);
}

/*
 * Encapsulate the callback function: Call the external entropy value callback function to clear
 * the entropy value (set the memory of the callback function to zero and release the buffer).
 */
WsecVoid WsecCleanupEntropy(unsigned char *entropyBuff, size_t buffLen)
{
    if (g_regCallbacks.rngCallbacks.cleanupEntropy != NULL) {
        g_regCallbacks.rngCallbacks.cleanupEntropy(entropyBuff, buffLen);
    }
}

/* Callback function encapsulation: Invoke the external UTC time callback to obtain the UTC time. */
WsecBool WsecGmTime(const time_t *curTime, struct tm *curTm)
{
    if (g_regCallbacks.timeCallbacks.gmTimeSafe == NULL) {
        return WSEC_FALSE;
    }
    return g_regCallbacks.timeCallbacks.gmTimeSafe(curTime, curTm);
}

/* Obtaining Additional Encryption Parameters */
unsigned long WsecHwGetEncExtraData(const unsigned char **extraData, unsigned int *extraLen)
{
    WSEC_ASSERT(extraData != NULL);
    WSEC_ASSERT(extraLen != NULL);

    if (g_regCallbacks.hardwareCallbacks.hwGetEncExtraData == NULL) {
        return WSEC_FAILURE;
    }
    return g_regCallbacks.hardwareCallbacks.hwGetEncExtraData(extraData, extraLen);
}

/* Obtaining Additional Decryption Parameters */
unsigned long WsecHwGetDecExtraData(const unsigned char **extraData, unsigned int *extraLen)
{
    WSEC_ASSERT(extraData != NULL);
    WSEC_ASSERT(extraLen != NULL);

    if (g_regCallbacks.hardwareCallbacks.hwGetDecExtraData == NULL) {
        return WSEC_FAILURE;
    }
    return g_regCallbacks.hardwareCallbacks.hwGetDecExtraData(extraData, extraLen);
}

/* Obtains the length of persistent data. */
unsigned long WsecHwGetPersistentDataLen(unsigned int *len)
{
    WSEC_ASSERT(len != NULL);
    if (g_regCallbacks.hardwareCallbacks.hwGetPersistentDataLen == NULL) {
        return WSEC_FAILURE;
    }
    *len = g_regCallbacks.hardwareCallbacks.hwGetPersistentDataLen();
    return WSEC_SUCCESS;
}

/* Initializing the Hardware Key Manager */
unsigned long WsecHwInitKeyMgr(const void *passthroughData, unsigned int passthroughDataLen)
{
    if (g_regCallbacks.hardwareCallbacks.hwInitKeyMgr == NULL) {
        return WSEC_FAILURE;
    }
    return g_regCallbacks.hardwareCallbacks.hwInitKeyMgr(passthroughData, passthroughDataLen);
}

/* Create a new key on the hardware. */
unsigned long WsecHwNewRootKey(unsigned char *persistentData, unsigned int *persistentDataLen, WsecHandle *handle)
{
    if (g_regCallbacks.hardwareCallbacks.hwNewRootKey == NULL) {
        return WSEC_FAILURE;
    }
    return g_regCallbacks.hardwareCallbacks.hwNewRootKey(persistentData, persistentDataLen, handle);
}

/* Loading Hardware Keys */
unsigned long WsecHwLoadRootkey(const unsigned char *persistentData, unsigned int persistentDataLen,
    WsecHandle *handle)
{
    if (g_regCallbacks.hardwareCallbacks.hwLoadRootkey == NULL) {
        return WSEC_FAILURE;
    }
    return g_regCallbacks.hardwareCallbacks.hwLoadRootkey(persistentData, persistentDataLen, handle);
}

/* Obtains the ciphertext length. */
unsigned long WsecHwGetCipherLen(unsigned int plaintextLen, unsigned int *ciphertextLen)
{
    if (g_regCallbacks.hardwareCallbacks.hwGetCipherLen == NULL) {
        return WSEC_FAILURE;
    }
    return g_regCallbacks.hardwareCallbacks.hwGetCipherLen(plaintextLen, ciphertextLen);
}

/* Encryption (software-layer root key or master key) */
unsigned long WsecHwEncData(WsecHandle handle, const unsigned char *extraData, unsigned int extraLen,
    const WsecPlainCipherBuffs *buffs)
{
    if (g_regCallbacks.hardwareCallbacks.hwEncData == NULL) {
        return WSEC_FAILURE;
    }
    return g_regCallbacks.hardwareCallbacks.hwEncData(handle, extraData, extraLen, buffs);
}

/* Decryption (software-layer root key or master key) */
unsigned long WsecHwDecData(WsecHandle handle, const unsigned char *extraData, unsigned int extraLen,
    const WsecPlainCipherBuffs *buffs)
{
    if (g_regCallbacks.hardwareCallbacks.hwDecData == NULL) {
        return WSEC_FAILURE;
    }
    return g_regCallbacks.hardwareCallbacks.hwDecData(handle, extraData, extraLen, buffs);
}

/* Unload hardware root key */
unsigned long WsecHwUnloadKey(WsecHandle handle)
{
    if (g_regCallbacks.hardwareCallbacks.hwUnloadKey == NULL) {
        return WSEC_FAILURE;
    }
    return g_regCallbacks.hardwareCallbacks.hwUnloadKey(handle);
}

/* Removing the Hardware Root Key */
unsigned long WsecHwRemoveKey(WsecHandle handle)
{
    if (g_regCallbacks.hardwareCallbacks.hwRemoveKey == NULL) {
        return WSEC_FAILURE;
    }
    return g_regCallbacks.hardwareCallbacks.hwRemoveKey(handle);
}

/* Deinitializes the hardware key manager. */
unsigned long WsecHwUninitKeyMgr(void)
{
    if (g_regCallbacks.hardwareCallbacks.hwUninitKeyMgr == NULL) {
        return WSEC_FAILURE;
    }
    return g_regCallbacks.hardwareCallbacks.hwUninitKeyMgr();
}
