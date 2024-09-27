/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Hardware root key management abstraction layer
 * Author: z00316590
 * Create: 2019-03-19
 * Notes: SRKs and HRKs are always synchronized (created, existing, and destroyed) and always bound.
 */
#include "kmcv3_rk.h"
#include "securec.h"
#include "cacv2_pri.h"
#include "kmcv3_maskinfo.h"
#include "wsecv2_callbacks.h"
#include "wsecv2_datetime.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_hash.h"
#include "wsecv2_util.h"
#include "wsecv2_mem.h"
#include "wsecv2_order.h"

/* Stores all loaded or created hardware root key memory data. */
static WsecArray g_hardRkMem = NULL;

/* Clears array elements. */
static void FreeKmcHardRkMem(KmcHardRkMem *hardRkMem)
{
    if (hardRkMem != NULL) {
        WSEC_CLEAR_FREE(hardRkMem->hardRk.hrkInfo.buff, hardRkMem->hardRk.hrkInfo.len);
        WSEC_CLEAR_FREE(hardRkMem->hardRk.srkInfo.buff, hardRkMem->hardRk.srkInfo.len);
        WSEC_CLEAR_FREE(hardRkMem, sizeof(KmcHardRkMem));
    }
}

/*
 * Callback function used to compare the sizes of two elements when
 * the KmcHardRkMem array is sorted or searched quickly.
 */
static int CompareHardRkMem(const WsecVoid *p1, const WsecVoid *p2)
{
    const KmcHardRkMem *rkA = NULL;
    const KmcHardRkMem *rkB = NULL;
    int i;

    WSEC_ASSERT(p1 != NULL);
    WSEC_ASSERT(p2 != NULL);
    /* HRKINFO and SRKINFO are used together to generate the key value. */
    rkA = (const KmcHardRkMem *)(*(const WsecVoid * const *)p1);
    rkB = (const KmcHardRkMem *)(*(const WsecVoid * const *)p2);
    if (rkA->hardRk.hrkInfo.len > rkB->hardRk.hrkInfo.len) {
        return WSEC_CMP_RST_BIG_THAN;
    }
    if (rkA->hardRk.hrkInfo.len < rkB->hardRk.hrkInfo.len) {
        return WSEC_CMP_RST_SMALL_THAN;
    }

    i = WSEC_MEMCMP(rkA->hardRk.hrkInfo.buff, rkB->hardRk.hrkInfo.buff, rkB->hardRk.hrkInfo.len);
    if (i > 0) {
        return WSEC_CMP_RST_BIG_THAN;
    }
    if (i < 0) {
        return WSEC_CMP_RST_SMALL_THAN;
    }

    if (rkA->hardRk.srkInfo.len > rkB->hardRk.srkInfo.len) {
        return WSEC_CMP_RST_BIG_THAN;
    }
    if (rkA->hardRk.srkInfo.len < rkB->hardRk.srkInfo.len) {
        return WSEC_CMP_RST_SMALL_THAN;
    }

    i = WSEC_MEMCMP(rkA->hardRk.srkInfo.buff, rkB->hardRk.srkInfo.buff, rkB->hardRk.srkInfo.len);
    if (i > 0) {
        return WSEC_CMP_RST_BIG_THAN;
    }
    if (i < 0) {
        return WSEC_CMP_RST_SMALL_THAN;
    }
    return WSEC_CMP_RST_EQUAL;
}

/* When the KmcHardRkMem array element is deleted, the data stored in the element is processed. */
static WsecVoid OnRemoveHardRkMem(WsecVoid *element, WsecUint32 elementSize)
{
    KmcHardRkMem *data = (KmcHardRkMem *)element;
    (void)elementSize;
    WSEC_ASSERT(elementSize == sizeof(KmcHardRkMem));

    FreeKmcHardRkMem(data);
}

/* New hardware root key memory */
static KmcHardRkMem *NewKmcHardRkMem(void)
{
    KmcHardRkMem *hardRkMem = NULL;
    unsigned int tmpLen;
    unsigned long ret;

    do {
        hardRkMem = (KmcHardRkMem *)WSEC_MALLOC(sizeof(KmcHardRkMem));
        if (hardRkMem == NULL) {
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        (void)memset_s(hardRkMem, sizeof(KmcHardRkMem), 0, sizeof(KmcHardRkMem));

        ret = WsecHwGetPersistentDataLen(&tmpLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("WsecHwGetPersistentDataLen failed %lu", ret);
            break;
        }
        hardRkMem->hardRk.hasHardRk = WSEC_TRUE;
        hardRkMem->hardRk.hrkInfo.buff = WSEC_MALLOC(tmpLen);
        if (hardRkMem->hardRk.hrkInfo.buff == NULL && tmpLen > 0) {
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        hardRkMem->hardRk.hrkInfo.len = tmpLen;
        ret = WsecHwGetCipherLen((unsigned int)sizeof(hardRkMem->key.maskedKey), &tmpLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("WsecHwGetCipherLen fail %lu", ret);
            break;
        }
        hardRkMem->hardRk.srkInfo.buff = WSEC_MALLOC(tmpLen);
        if (hardRkMem->hardRk.srkInfo.buff == NULL && tmpLen > 0) {
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        hardRkMem->hardRk.srkInfo.len = tmpLen;
        ret = CacRandom(hardRkMem->key.maskedKey, (unsigned int)sizeof(hardRkMem->key.maskedKey));
        if (ret != WSEC_SUCCESS) {
            break;
        }
        hardRkMem->refCount = 1;
    } while (0);
    if (ret != WSEC_SUCCESS && hardRkMem != NULL) {
        WSEC_CLEAR_FREE(hardRkMem->hardRk.hrkInfo.buff, hardRkMem->hardRk.hrkInfo.len);
        WSEC_CLEAR_FREE(hardRkMem->hardRk.srkInfo.buff, hardRkMem->hardRk.srkInfo.len);
        WSEC_CLEAR_FREE(hardRkMem, sizeof(KmcHardRkMem));
        WSEC_LOG_E1("Get new KmcHardRkMem failed, ret %lu", ret);
        hardRkMem = NULL;
    }
    return hardRkMem;
}

/* Cloning the Hardware Root Key Memory */
static unsigned long FillKsfHardRk(const KmcKsfHardRk *src, KmcKsfHardRk *dest)
{
    WSEC_ASSERT(src != NULL);
    WSEC_ASSERT(dest != NULL);
    WSEC_ASSERT(dest->hrkInfo.buff == NULL);
    WSEC_ASSERT(dest->srkInfo.buff == NULL);
    dest->hrkInfo.buff = WSEC_CLONE_BUFF(src->hrkInfo.buff, src->hrkInfo.len);
    if (dest->hrkInfo.buff == NULL && src->hrkInfo.len > 0) {
        return WSEC_ERR_MALLOC_FAIL;
    }
    dest->hrkInfo.len = src->hrkInfo.len;
    dest->srkInfo.buff = WSEC_CLONE_BUFF(src->srkInfo.buff, src->srkInfo.len);
    if (dest->srkInfo.buff == NULL && src->srkInfo.len > 0) {
        WSEC_FREE(dest->hrkInfo.buff);
        return WSEC_ERR_MALLOC_FAIL;
    }
    dest->hasHardRk = src->hasHardRk;
    dest->srkInfo.len = src->srkInfo.len;
    return WSEC_SUCCESS;
}

/* Clear hardware root key memory resources. */
static void ClearKsfHardRk(KmcKsfHardRk *hardRk)
{
    WSEC_ASSERT(hardRk != NULL);
    WSEC_CLEAR_FREE(hardRk->hrkInfo.buff, hardRk->hrkInfo.len);
    WSEC_CLEAR_FREE(hardRk->srkInfo.buff, hardRk->srkInfo.len);
}

/* Search for the root key that has been loaded or created in the memory. */
static KmcHardRkMem *FindHardRkMem(const KmcKsfHardRk *hardRk, int *pos)
{
    int count = WsecArrGetCount(g_hardRkMem);
    int idx;
    int i;
    KmcHardRkMem *ret = NULL;
    KmcHardRkMem *one = NULL;
    if (hardRk == NULL || hardRk->hrkInfo.buff == NULL || hardRk->srkInfo.buff == NULL) {
        return NULL;
    }
    for (idx = 0; idx < count; idx++) {
        one = (KmcHardRkMem *)WsecArrGetAt(g_hardRkMem, idx);
        if (one == NULL) {
            continue;
        }
        if (one->hardRk.hrkInfo.len != hardRk->hrkInfo.len) {
            continue;
        }
        i = WSEC_MEMCMP(one->hardRk.hrkInfo.buff, hardRk->hrkInfo.buff, hardRk->hrkInfo.len);
        if (i != 0) {
            continue;
        }
        /*
         * The SRK must be considered. Otherwise, when the hardware-layer key is fixed (for example, the
         * TPM maintains the fixed key, the new key is not implemented, and hrkInfo remains unchanged).
         * If a software-layer root key exists and both files are damaged,
         * the KSF generated using CheckKsf will be written into the new SRK file,
         * However, the old SRK is still used for encrypting the MK.
         * As a result, the file cannot be restored after being damaged.
         * Condition 1: The software layer root key exists.Condition 2: The hardware layer returns a fixed HRKINFO
         * in any case, that is, only one fixed HRK is maintained.
         */
        if (one->hardRk.srkInfo.len != hardRk->srkInfo.len) {
            continue;
        }
        i = WSEC_MEMCMP(one->hardRk.srkInfo.buff, hardRk->srkInfo.buff, hardRk->srkInfo.len);
        if (i != 0) {
            continue;
        }
        ret = one;
        if (pos != NULL) {
            *pos = idx;
        }
        break;
    }
    return ret;
}

/*
 * Initialize the hardware root key environment and invoke the hardware adaptation
 * layer interface to initialize the hardware environment.
 */
unsigned long KmcHardRkInit(const WsecVoid *hardwareParam, WsecUint32 hardwareParamLen)
{
    g_hardRkMem = WsecArrInitialize(0, (WsecUint32)sizeof(KmcHardRkMem), 0, CompareHardRkMem, OnRemoveHardRkMem);
    if (g_hardRkMem == NULL) {
        WSEC_LOG_E("WsecArrInitialize failed.");
        return WSEC_ERR_OPER_ARRAY_FAIL;
    }

    return WsecHwInitKeyMgr(hardwareParam, hardwareParamLen);
}

/* Assign KmcKsfHardRk to another buffer. Note that reference counting does not need to be changed. */
void KmcAssignKsfHardRk(KmcKsfHardRk *src, KmcKsfHardRk *dest)
{
    WSEC_ASSERT(src != NULL);
    WSEC_ASSERT(dest != NULL);
    (void)memcpy_s(dest, sizeof(KmcKsfHardRk), src, sizeof(KmcKsfHardRk));
    (void)memset_s(src, sizeof(KmcKsfHardRk), 0, sizeof(KmcKsfHardRk));
}

/* KmcKsfHardRk clone. The value of reference counting increases by 1. */
unsigned long KmcHardRkCloneMember(const KmcKsfHardRk *src, KmcKsfHardRk *dest)
{
    KmcHardRkMem *hardRkMem = FindHardRkMem(src, NULL);
    unsigned long ret;
    WSEC_ASSERT(src != NULL);
    WSEC_ASSERT(dest != NULL);
    if (src->hasHardRk == WSEC_FALSE) {
        return WSEC_SUCCESS;
    }
    if (hardRkMem == NULL) {
        return WSEC_ERR_KMC_HARDWARE_RK_NOT_FOUND;
    }

    ret = FillKsfHardRk(src, dest);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_W1("FillKsfHardRk failed %lu", ret);
        return ret;
    }
    hardRkMem->refCount++;
    return WSEC_SUCCESS;
}

/* Creating a Hardware Root Key */
unsigned long KmcHardRkNew(KmcKsfHardRk *hardRk)
{
    KmcHardRkMem *hardRkMem = NewKmcHardRkMem();
    unsigned long ret;
    int arrRet;
    unsigned long tmpRet;
    WSEC_ASSERT(hardRk != NULL);
    do {
        if (hardRkMem == NULL) {
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        ret = WsecHwNewRootKey((unsigned char *)(hardRkMem->hardRk.hrkInfo.buff), &hardRkMem->hardRk.hrkInfo.len, &hardRkMem->hwRkHandle);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("WsecHwNewRootKey failed %lu", ret);
            break;
        }
        ret = WsecKmcHwEncData(hardRkMem->hwRkHandle, hardRkMem->key.maskedKey,
            (unsigned int)sizeof(hardRkMem->key.maskedKey),
            (unsigned char *)(hardRkMem->hardRk.srkInfo.buff), &hardRkMem->hardRk.srkInfo.len);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E2("WsecKmcHwEncData failed %s %lu", __FUNCTION__, ret);
            break;
        }
        ret = ProtectDataSameBuf(hardRkMem->key.maskedKey, (unsigned int)sizeof(hardRkMem->key.maskedKey));
        if (ret != WSEC_SUCCESS) {
            break;
        }
        arrRet = WsecArrAddOrderly(g_hardRkMem, hardRkMem);
        if (arrRet < 0) {
            ret = WSEC_ERR_OPER_ARRAY_FAIL;
            break;
        }
        ret = FillKsfHardRk(&hardRkMem->hardRk, hardRk);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        hardRkMem->refCount = 1;
    } while (0);
    if (ret != WSEC_SUCCESS) {
        if (hardRkMem != NULL && hardRkMem->hwRkHandle != NULL) {
            tmpRet = WsecHwRemoveKey(hardRkMem->hwRkHandle);
            if (tmpRet != WSEC_SUCCESS) {
                WSEC_LOG_E1("WsecHwRemoveKey fail, ret is %lu", tmpRet);
            }
        }
        FreeKmcHardRkMem(hardRkMem);
        ClearKsfHardRk(hardRk);
    }
    return ret;
}

/* Load the hardware root key. After the loading, the initial count is 1. */
static unsigned long HardRkLoadNewHardRkMem(const KmcKsfHardRk *hardRk)
{
    KmcHardRkMem *hardRkMem = NULL;
    unsigned long ret;
    unsigned int decLen;
    int idx;
    unsigned long tmpRet;
    do {
        hardRkMem = (KmcHardRkMem *)WSEC_MALLOC(sizeof(KmcHardRkMem));
        if (hardRkMem == NULL) {
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }

        ret = FillKsfHardRk(hardRk, &hardRkMem->hardRk);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = WsecHwLoadRootkey((const unsigned char *)(hardRkMem->hardRk.hrkInfo.buff), hardRkMem->hardRk.hrkInfo.len, &hardRkMem->hwRkHandle);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E2("%s WsecHwLoadRootkey() failed %lu", __FUNCTION__, ret);
            break;
        }
        decLen = sizeof(hardRkMem->key.maskedKey);
        ret = WsecKmcHwDecData(hardRkMem->hwRkHandle, (const unsigned char *)(hardRkMem->hardRk.srkInfo.buff), hardRkMem->hardRk.srkInfo.len,
            hardRkMem->key.maskedKey, &decLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E2("%s WsecKmcHwDecData() failed %lu", __FUNCTION__, ret);
            break;
        }
        ret = ProtectDataSameBuf(hardRkMem->key.maskedKey, (unsigned int)sizeof(hardRkMem->key.maskedKey));
        if (ret != WSEC_SUCCESS) {
            break;
        }
        idx = WsecArrAddOrderly(g_hardRkMem, hardRkMem);
        if (idx < 0) {
            ret = WSEC_ERR_OPER_ARRAY_FAIL;
            break;
        }
        hardRkMem->refCount = 1;
    } while (0);
    if (ret != WSEC_SUCCESS) {
        if (hardRkMem != NULL && hardRkMem->hwRkHandle != NULL) {
            tmpRet = WsecHwUnloadKey(hardRkMem->hwRkHandle);
            if (tmpRet != WSEC_SUCCESS) {
                WSEC_LOG_E1("WsecHwUnloadKey fail, ret is %lu", tmpRet);
            }
        }
        FreeKmcHardRkMem(hardRkMem);
    }
    return ret;
}

/*
 * Loads the hardware root key. If the hardware root key has been loaded, the counter increases by 1.
 * Otherwise, the hardware root key is loaded.
 */
unsigned long KmcHardRkLoad(const KmcKsfHardRk *hardRk)
{
    KmcHardRkMem *hardRkMem = FindHardRkMem(hardRk, NULL);
    WSEC_ASSERT(hardRk != NULL);
    if (hardRkMem != NULL) {
        hardRkMem->refCount++;
        return WSEC_SUCCESS;
    }
    return HardRkLoadNewHardRkMem(hardRk);
}

/*
 * The hardware root key is uninstalled only from the memory of the adaptation layer and is not deleted.
 * If the persistent information is obtained next time, the root key can be loaded.
 */
static unsigned long KmcHardRkUnload(const KmcKsfHardRk *hardRk)
{
    int idx;
    KmcHardRkMem *hardRkMem = FindHardRkMem(hardRk, &idx);
    unsigned long ret;
    if (hardRkMem == NULL) {
        return WSEC_ERR_KMC_HARDWARE_RK_NOT_FOUND;
    }
    if (hardRkMem->refCount > 0) {
        hardRkMem->refCount--;
    }
    ret = WSEC_SUCCESS;
    if (hardRkMem->refCount == 0) {
        ret = WsecHwUnloadKey(hardRkMem->hwRkHandle);
        WSEC_LOG_I2("Unload hardware rk ret %lu, refCount is %u.", ret, hardRkMem->refCount);
        WsecArrRemoveAt(g_hardRkMem, idx);
    }

    return ret;
}

/*
 * Remove the hardware root key. Note that the count is processed first.
 * If the count is 0, the hardware root key is removed.
 */
WsecVoid KmcHardRkRemove(const KmcKsfHardRk *hardRk)
{
    int pos;
    KmcHardRkMem *hardRkMem = FindHardRkMem(hardRk, &pos);
    unsigned long ret;
    if (hardRkMem == NULL) {
        return;
    }

    if (hardRkMem->refCount > 0) {
        hardRkMem->refCount--;
    }
    if (hardRkMem->refCount == 0) {
        ret = WsecHwRemoveKey(hardRkMem->hwRkHandle);
        WSEC_LOG_I2("Remove hardware rk ret %lu, refCount is %u.", ret, hardRkMem->refCount);
        WsecArrRemoveAt(g_hardRkMem, pos);
    }
}

/* Delete the hardware root key by decreasing reference counting. The root key can be deleted when the count is 0. */
static WsecVoid RemoveKsfHardRk(const KmcKsfHardRk *hardRk)
{
    WSEC_ASSERT(hardRk != NULL);
    if (hardRk->hasHardRk == WSEC_TRUE) {
        KmcHardRkRemove(hardRk);
    }
}

/*
 * KmcKsfHardRk indicates the root key. (The root key is removed when Update, RewriteOnCheckfail, or Erase is executed.
 * The root key is not removed when a new key is created because it is empty.)
 */
WsecVoid KmcKsfHardRkRmvFree(KmcKsfHardRk *hardRk)
{
    RemoveKsfHardRk(hardRk);
    WSEC_CLEAR_FREE(hardRk->hrkInfo.buff, hardRk->hrkInfo.len);
    hardRk->hrkInfo.len = 0;
    WSEC_CLEAR_FREE(hardRk->srkInfo.buff, hardRk->srkInfo.len);
    hardRk->srkInfo.len = 0;
    hardRk->hasHardRk = WSEC_FALSE;
}

/*
 * The unload hardware root key does not release the memory.
 * The root key can still be loaded next time based on persistent information.
 */
static WsecVoid KsfHardRkUnload(const KmcKsfHardRk *hardRk)
{
    unsigned long ret;
    WSEC_ASSERT(hardRk != NULL);
    if (hardRk->hasHardRk == WSEC_TRUE) {
        ret = KmcHardRkUnload(hardRk);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("Unload KsfHardRk failed %lu", ret);
        }
    }
}

/*
 * Unload the hardware root key and release the memory.
 * The root key can still be loaded next time based on persistent information.
 */
WsecVoid KmcKsfHardRkUnloadFree(KmcKsfHardRk *hardRk)
{
    KsfHardRkUnload(hardRk);
    WSEC_CLEAR_FREE(hardRk->hrkInfo.buff, hardRk->hrkInfo.len);
    hardRk->hrkInfo.len = 0;
    WSEC_CLEAR_FREE(hardRk->srkInfo.buff, hardRk->srkInfo.len);
    hardRk->srkInfo.len = 0;
    hardRk->hasHardRk = WSEC_FALSE;
}

/*
 * Use the hardware root key to encrypt information, including the software-layer root key and KSF HMAC KEY.
 * If the software-layer root key is not used, the MK needs to be encrypted.
 */
unsigned long KmcEncByHrk(const KmcKsfHardRk *hardRk,
    const unsigned char *plaintext, unsigned int plainLen,
    unsigned char *ciphertext, unsigned int *cipherLen)
{
    unsigned long ret;
    KmcHardRkMem *hardRkMem = FindHardRkMem(hardRk, NULL);
    if (hardRkMem == NULL) {
        return WSEC_ERR_KMC_HARDWARE_RK_NOT_FOUND;
    }
    ret = WsecKmcHwEncData(hardRkMem->hwRkHandle, plaintext, plainLen, ciphertext, cipherLen);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("WsecKmcHwEncData failed %lu", ret);
    }
    return ret;
}

/*
 * Use the hardware root key to decrypt information, including the software-layer root key and KSF HMAC KEY.
 * If the software-layer root key is not used, the MK needs to be decrypted.
 */
unsigned long KmcDecByHrk(const KmcKsfHardRk *hardRk,
    const unsigned char *ciphertext, unsigned int cipherLen,
    unsigned char *plaintext, unsigned int *plainLen)
{
    unsigned long ret;
    KmcHardRkMem *hardRkMem = FindHardRkMem(hardRk, NULL);
    if (hardRkMem == NULL) {
        return WSEC_ERR_KMC_HARDWARE_RK_NOT_FOUND;
    }
    ret = WsecKmcHwDecData(hardRkMem->hwRkHandle, ciphertext, cipherLen, plaintext, plainLen);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("WsecKmcHwDecData failed %lu", ret);
    }
    return ret;
}

/* Obtaining Software-Layer Keys (Secure Hardware Protection) */
static unsigned long GetAllKey(const KmcKsfHardRk *hardRk, KmcMaskedKey *allKey)
{
    KmcHardRkMem *hardRkMem = FindHardRkMem(hardRk, NULL);
    unsigned long ret;
    unsigned int allKeyLen = sizeof(KmcMaskedKey);
    if (hardRkMem == NULL) {
        WSEC_LOG_E1("%s rk not found", __FUNCTION__);
        return WSEC_ERR_KMC_HARDWARE_RK_NOT_FOUND;
    }
    ret = UnprotectData(hardRkMem->key.maskedKey, (unsigned int)sizeof(hardRkMem->key.maskedKey),
        allKey->maskedKey, &allKeyLen);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("GetAllKey UnprotectData failed %lu", ret);
    }
    return ret;
}

/* Encrypt the data in the temporary memory. */
static unsigned long KmcHardRkPreReEnc(const KmcHardRkMem *hardRkMem, KmcKsfHardRk *tempSrc, KmcKsfHardRk *tempDest)
{
    unsigned long ret;
    KmcMaskedKey allKey;
    WsecBuff tempSrkInfo = { NULL, 0 };
    unsigned int allKeyLen = sizeof(KmcMaskedKey);

    do {
        ret = UnprotectData(hardRkMem->key.maskedKey, (unsigned int)sizeof(hardRkMem->key.maskedKey),
            allKey.maskedKey, &allKeyLen);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = WsecHwGetCipherLen((unsigned int)sizeof(allKey.maskedKey), &allKeyLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E2("WsecHwGetCipherLen failed %s %lu", __FUNCTION__, ret);
            break;
        }
        tempSrkInfo.buff = WSEC_MALLOC(allKeyLen);
        if (tempSrkInfo.buff == NULL && allKeyLen > 0) {
            WSEC_LOG_E2("KmcHardRkReEncSrk malloc %s %u", __FUNCTION__, allKeyLen);
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        tempSrkInfo.len = allKeyLen;
        ret = WsecKmcHwEncData(hardRkMem->hwRkHandle, allKey.maskedKey, (unsigned int)sizeof(allKey.maskedKey),
            (unsigned char *)(tempSrkInfo.buff), &tempSrkInfo.len);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E2("WsecKmcHwEncData failed %s %lu", __FUNCTION__, ret);
            break;
        }
        (void)memcpy_s(tempSrc, sizeof(KmcKsfHardRk), &hardRkMem->hardRk, sizeof(KmcKsfHardRk));
        tempSrc->srkInfo.buff = tempSrkInfo.buff;
        tempSrc->srkInfo.len = tempSrkInfo.len;
        ret = FillKsfHardRk(tempSrc, tempDest);
        if (ret != WSEC_SUCCESS) {
            break;
        }
    } while (0);
    if (ret != WSEC_SUCCESS) {
        WSEC_CLEAR_FREE(tempSrkInfo.buff, tempSrkInfo.len);
        WSEC_CLEAR_FREE(tempDest->hrkInfo.buff, tempDest->hrkInfo.len);
        WSEC_CLEAR_FREE(tempDest->srkInfo.buff, tempDest->srkInfo.len);
    }
    (void)memset_s(allKey.maskedKey, sizeof(allKey.maskedKey), 0, sizeof(allKey.maskedKey));
    return ret;
}

/* For hardware-based root keys, KSF V3 re-encrypts software-layer root keys. */
unsigned long KmcHardRkReEncSrk(KmcKsfHardRk *hardRk)
{
    KmcHardRkMem *hardRkMem = FindHardRkMem(hardRk, NULL);
    KmcKsfHardRk tempDest = { WSEC_FALSE, { NULL, 0 }, { NULL, 0 } };
    KmcKsfHardRk tempSrc = { WSEC_FALSE, { NULL, 0 }, { NULL, 0 } };
    unsigned long ret;
    if (hardRkMem == NULL) {
        WSEC_LOG_E1("%s rk not found", __FUNCTION__);
        return WSEC_ERR_KMC_HARDWARE_RK_NOT_FOUND;
    }
    ret = KmcHardRkPreReEnc(hardRkMem, &tempSrc, &tempDest);
    if (ret == WSEC_SUCCESS) {
        /*
         * If the operation is successful, the original srkInfo.buff is released because it has
         * been replaced by the new tempSrc.srkInfo.buff and the new ciphertext is stored.
         */
        WSEC_CLEAR_FREE(hardRkMem->hardRk.srkInfo.buff, hardRkMem->hardRk.srkInfo.len);
        (void)memcpy_s(&hardRkMem->hardRk, sizeof(KmcKsfHardRk), &tempSrc, sizeof(KmcKsfHardRk));
        /*
         * If the operation is successful, release the original ardRk->hrkInfo.buff and hardRk->srkInfo.buff files
         * and use the tempDest cloned information.
         */
        WSEC_CLEAR_FREE(hardRk->hrkInfo.buff, hardRk->hrkInfo.len);
        WSEC_CLEAR_FREE(hardRk->srkInfo.buff, hardRk->srkInfo.len);
        (void)memcpy_s(hardRk, sizeof(KmcKsfHardRk), &tempDest, sizeof(KmcKsfHardRk));
    }
    return WSEC_SUCCESS;
}

/* Encrypt the MK using the root key of the software layer. */
unsigned long KmcEncBySrk(const KmcKsfHardRk *hardRk,
    const WsecUint32 algId,
    const unsigned char *iv, unsigned int ivLen,
    const WsecBuffConst *plainBuff,
    unsigned char *ciphertext, unsigned int *cipherLen)
{
    KmcMaskedKey realKey;
    unsigned long ret = GetAllKey(hardRk, &realKey);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    ret = CacEncrypt(algId, realKey.tempName.softLevelRk, (WsecUint32)sizeof(realKey.tempName.softLevelRk),
        iv, ivLen, plainBuff->buff, plainBuff->len, ciphertext, cipherLen);
    (void)memset_s(&realKey, sizeof(realKey), 0, sizeof(realKey));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("KmcEncBySrk CacEncrypt failed %lu", ret);
    }
    return ret;
}

/* Decrypt the MK using the software-layer root key. */
unsigned long KmcDecBySrk(const KmcKsfHardRk *hardRk,
    const WsecUint32 algId,
    const unsigned char *iv, unsigned int ivLen,
    const WsecBuffConst *cipherBuff,
    unsigned char *plaintext, unsigned int *plainLen)
{
    KmcMaskedKey realKey;
    unsigned long ret = GetAllKey(hardRk, &realKey);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    ret = CacDecrypt(algId, realKey.tempName.softLevelRk, (WsecUint32)sizeof(realKey.tempName.softLevelRk),
        iv, ivLen, cipherBuff->buff, cipherBuff->len, plaintext, plainLen);
    (void)memset_s(&realKey, sizeof(realKey), 0, sizeof(realKey));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("KmcDecBySrk CacDecrypt failed %lu", ret);
    }
    return ret;
}

/* Calculating the KSFHMAC using the KSF HMAC key */
unsigned long KmcHardRkGetKsfHmac(const KmcKsfHardRk *hardRk,
    WsecUint32 alg,
    KmcKsfHmac *ksfHmac,
    WsecUint32 hashLen,
    WsecUint32 *hmacLen)
{
    KmcMaskedKey realKey;
    unsigned long ret = GetAllKey(hardRk, &realKey);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    WSEC_ASSERT(ksfHmac != NULL);
    WSEC_ASSERT(hmacLen != NULL);
    ret = CacHmac(alg, realKey.tempName.ksfHmacKey, (WsecUint32)sizeof(realKey.tempName.ksfHmacKey),
        ksfHmac->HashData.hashData, hashLen, ksfHmac->HashHmac.hashHmac, hmacLen);
    (void)memset_s(&realKey, sizeof(KmcMaskedKey), 0, sizeof(KmcMaskedKey));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("KmcHardRkGetKsfHmac CacHmac %lu", ret);
    }
    return ret;
}

/* Deinitialize the hardware root key adaptation layer. */
unsigned long KmcHardRkUninit(void)
{
    int count = WsecArrGetCount(g_hardRkMem);
    int i;
    KmcHardRkMem *hardRkMem = NULL;
    unsigned long ret;
    for (i = 0; i < count; i++) {
        hardRkMem = (KmcHardRkMem *)WsecArrGetAt(g_hardRkMem, i);
        if (hardRkMem != NULL) {
            WSEC_LOG_W1("KmcHardRkUninit hardware root key exist [%u] (should be unloaded)", hardRkMem->refCount);
            ret = WsecHwUnloadKey(hardRkMem->hwRkHandle);
            if (ret != WSEC_SUCCESS) {
                WSEC_LOG_E1("KmcHardRkUninit WsecHwUnloadKey fail return value %lu", ret);
            }
        }
    }
    g_hardRkMem = WsecArrFinalize(g_hardRkMem);
    ret = WsecHwUninitKeyMgr();
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("WsecHwUninitKeyMgr failed with %lu", ret);
    }
    return ret;
}

/*
 * Use the memory protection technology to remove the protection software-layer root
 * key and KSF HMAC key from the memory.
 */
unsigned long KmcUnprotectDataForMaskedKey(void)
{
    int count = WsecArrGetCount(g_hardRkMem);
    int i;
    KmcHardRkMem *hardRkMem = NULL;
    unsigned long ret;
    for (i = 0; i < count; i++) {
        hardRkMem = (KmcHardRkMem *)WsecArrGetAt(g_hardRkMem, i);
        if (hardRkMem != NULL) {
            ret = UnprotectDataSameBuf(hardRkMem->key.maskedKey, (unsigned int)sizeof(hardRkMem->key.maskedKey));
            if (ret != WSEC_SUCCESS) {
                return ret;
            }
        }
    }
    return WSEC_SUCCESS;
}

/* Protecting software-layer root keys and KSF HMAC keys using memory protection technologies */
unsigned long KmcProtectDataForMaskedKey(void)
{
    int count = WsecArrGetCount(g_hardRkMem);
    int i;
    KmcHardRkMem *hardRkMem = NULL;
    unsigned long ret;
    for (i = 0; i < count; i++) {
        hardRkMem = (KmcHardRkMem *)WsecArrGetAt(g_hardRkMem, i);
        if (hardRkMem != NULL) {
            ret = ProtectDataSameBuf(hardRkMem->key.maskedKey, (unsigned int)sizeof(hardRkMem->key.maskedKey));
            if (ret != WSEC_SUCCESS) {
                return ret;
            }
        }
    }
    return WSEC_SUCCESS;
}

/* Converts the byte order of the RK information in the KSF. */
WsecVoid CvtByteOrderForKsfRk(KmcKsfRk *rk, WsecUint32 direction, WsecUint16 ksfVersion)
{
    KmcRkAttributes *data = NULL;
    WSEC_ASSERT(WSEC_IS2(direction, WBCHOST2NETWORK, WBCNETWORK2HOST));
    WSEC_ASSERT(IsValidKsfVersion(ksfVersion) == WSEC_TRUE);
    if (rk == NULL) {
        return;
    }

    data = &(rk->rkAttributes);

    data->version = WSEC_BYTE_ORDER_CVT_S(direction, data->version);
    data->rkMaterialFrom = WSEC_BYTE_ORDER_CVT_S(direction, data->rkMaterialFrom);
    data->rmkIter = WSEC_BYTE_ORDER_CVT_L(direction, data->rmkIter);
    rk->mkNum = WSEC_BYTE_ORDER_CVT_L(direction, rk->mkNum);

    /*
     * updateCounter is a new field in KMC V2 SPC002.
     * mkRecordLen sharedMkUpdateCounter is a new field in KMC 3.0.
     * These fields are stored in the network sequence in KSF V2 and KSF 3.0
     * and are not used in KMC V2 SPC002 and earlier versions.
     * Consider the version downgrade scenario
     * KSF V2 and KSF 3.0 are used for conversion. The rollback from KMC 3.0 to any KMC V2 version is not affected
     * because the KMC V2 version (including versions earlier than SPC002) uses the network sequence for verification.
     * In the KSF V1 scenario, the host order is retained. When the KMC is downgraded from 3.0 to V1, the downgrade is
     * not affected because the host order is verified during V1 verification.
     */
    if (IsKsfV2(ksfVersion) == WSEC_TRUE || IsKsfV3(ksfVersion) == WSEC_TRUE) {
        rk->updateCounter = WSEC_BYTE_ORDER_CVT_L(direction, rk->updateCounter);
        rk->mkRecordLen = WSEC_BYTE_ORDER_CVT_L(direction, rk->mkRecordLen);
        rk->sharedMkUpdateCounter = WSEC_BYTE_ORDER_CVT_L(direction, rk->sharedMkUpdateCounter);
    }
    WsecCvtByteOrderForDateTime(&(data->rkCreateTimeUtc), direction);
    WsecCvtByteOrderForDateTime(&(data->rkExpiredTimeUtc), direction);
}

/*
 * Create RK parameters. The generated result is valid only in V1 and V2.
 * In V3, the hardware root key is used to protect the MK or SRK.
 */
static unsigned long CreateRkParameters(const WsecBuffConst *entropy, KmcRkParameters *keyParameters)
{
    unsigned char allMaterial[MATERIAL_COUNT][KMC_MATERIAL_SIZE];
    unsigned char materialSerial[2] = { 0x01, 0x02 }; /* 2 groups of secure random number are generated. */
    /* 4 buffers are used for calculating the hash value. */
    WsecBuffConst buffArray[4] = { { NULL, 0 }, { NULL, 0 }, { NULL, 0 }, { NULL, 0 } };
    WsecBuff wriHash[2] = { { NULL, 0 }, { NULL, 0 } }; /* 2 key material buffers */
    int i;
    unsigned long ret = WSEC_SUCCESS;
    WsecUint32 buffNum;
    int buffIndexBase = 0;
    WSEC_ASSERT(keyParameters != NULL);
    if (CacRandom(keyParameters->rmkSalt, (WsecUint32)sizeof(keyParameters->rmkSalt)) != WSEC_SUCCESS) {
        WSEC_LOG_E("CacRandom() failed.");
        return WSEC_ERR_GET_RAND_FAIL;
    }
    /*
     * 2. Keystore stores two root key materials.
     * Assuming that two random numbers R1 and R2 and an external entropy E,
     * two root key materials M1 and M2 are generated according to the following algorithm
     * the M1 = SHA256 of (0x1 + R1 + R2 + E)
     * the M2 = SHA256 of (0x2 + R1 + R2 + E)
     */
    if (CacRandom(allMaterial, (WsecUint32)sizeof(allMaterial)) != WSEC_SUCCESS) {
        WSEC_LOG_E("CacRandom() failed.");
        return WSEC_ERR_GET_RAND_FAIL;
    }
    buffIndexBase++;
    WSEC_BUFF_ASSIGN(buffArray[buffIndexBase], allMaterial[0], KMC_MATERIAL_SIZE); /* secure random number 1 */
    buffIndexBase++;
    WSEC_BUFF_ASSIGN(buffArray[buffIndexBase], allMaterial[1], KMC_MATERIAL_SIZE); /* secure random number 2 */
    if (entropy != NULL) {
        buffIndexBase++;
        /* The external entropy input is stored in index 3. */
        WSEC_BUFF_ASSIGN(buffArray[buffIndexBase], entropy->buff, entropy->len);
    }
    buffNum = ((WsecUint32)1 + MATERIAL_COUNT + (entropy != NULL ? (WsecUint32)1 : (WsecUint32)0));
    WSEC_BUFF_ASSIGN(wriHash[0], keyParameters->rkMaterialA, KMC_MATERIAL_SIZE);
    WSEC_BUFF_ASSIGN(wriHash[1], keyParameters->rkMaterialB, KMC_MATERIAL_SIZE);

    /* Generate root key materials. */
    for (i = 0; i < MATERIAL_COUNT; i++) {
        WSEC_BUFF_ASSIGN(buffArray[0], &materialSerial[i], sizeof(unsigned char));
        if (!WsecCreateHashCode(WSEC_ALGID_SHA256, buffArray, buffNum, &wriHash[i])) {
            WSEC_LOG_E("WsecCreateHashCode() fail.");
            ret = WSEC_ERR_GEN_HASH_CODE_FAIL;
            break;
        }
    }
    (void)memset_s(allMaterial, sizeof(allMaterial), 0, sizeof(allMaterial));
    return ret;
}

/* Creating a Root Key File */
unsigned long KmcCreateRootKey(const WsecBuffConst *entropy,
    WsecUint16 ksfVersion,
    const KmcCfg *kmcCfg,
    KmcKsfRk *rk,
    KmcKsfHardRk *hardRk)
{
    KmcRkAttributes *keyAttributes = NULL;
    unsigned long ret; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    if (rk == NULL) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_ERR_INVALID_ARG;
    }
    if (kmcCfg == NULL) {
        WSEC_LOG_E("The config memory does not exist");
        return WSEC_ERR_KMC_KEYCFGMEM_NOTEXIST;
    }
    keyAttributes = &rk->rkAttributes;

    /* 1. Basic Information */
    keyAttributes->version = ksfVersion;
    keyAttributes->rkMaterialFrom = KMC_RK_GEN_BY_INNER;
    ret = SetLifeTime(kmcCfg->rkCfg.validity,
        &keyAttributes->rkCreateTimeUtc, &keyAttributes->rkExpiredTimeUtc);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("SetLifeTime failed %lu", ret);
        return ret;
    }
    keyAttributes->rmkIter = kmcCfg->rkCfg.rmkIter;

    ret = CreateRkParameters(entropy, &rk->rkParameters);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("CreateRkParameters failed %lu", ret);
        return ret;
    }
    if (IsKsfV3(ksfVersion) == WSEC_TRUE) {
        ret = KmcHardRkNew(hardRk);
        if (ret != WSEC_SUCCESS) {
            return ret;
        }
    }
    WSEC_LOG_I("Root Key is generated.");
    return ret;
}

/* Verifying the RK Hash Value to Obtain the Version Number */
unsigned long KmcCheckRk(KmcKsfRk *rk, WsecUint16 ksfVersion)
{
    WsecBuffConst temp = { NULL, 0 };
    unsigned long ret;
    /* 1) Check file version number */
    if (IsValidKsfVersion(ksfVersion) == WSEC_FALSE) {
        WSEC_LOG_E1("KSF version %hu is not correct.", ksfVersion);
        return WSEC_ERR_KMC_KSF_DATA_INVALID;
    }

    /* 2) Check whether the number of MKs exceeds the threshold. */
    if (rk->mkNum > (WsecUint32)WSEC_MK_NUM_MAX) {
        WSEC_LOG_E2("KSF mkNum (%u) invalid, (MAX = %u).", rk->mkNum, WSEC_MK_NUM_MAX);
        return WSEC_ERR_KMC_MK_NUM_OVERFLOW;
    }

    /* 3) Check the hash value. */
    if (CalcByNetWorkOrder(ksfVersion) == WSEC_TRUE) {
        CvtByteOrderForKsfRk(rk, WBCHOST2NETWORK, ksfVersion);
    }
    WSEC_BUFF_ASSIGN(temp, rk, (sizeof(KmcKsfRk) - sizeof(rk->aboveHash)));
    ret = WsecCheckIntegrity(WSEC_ALGID_SHA256, &temp, 1, rk->aboveHash, (WsecUint32)sizeof(rk->aboveHash));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("KSF header integrity error.");
    }
    if (CalcByNetWorkOrder(ksfVersion) == WSEC_TRUE) {
        CvtByteOrderForKsfRk(rk, WBCNETWORK2HOST, ksfVersion);
    }
    return ret;
}

/*
 * Obtaining Hard-coded Root Key Materials
 * To improve the security of root key materials in a pure software environment,
 * two of the three materials are stored in the Keystore,
 * The other material is obtained through hard coding.
 */
static WsecVoid GetRkMeterial(unsigned char *material, size_t size)
{
    unsigned char randNum[KMC_MATERIAL_SIZE] = {
        0xB2, 0xA1, 0x0C, 0x73, 0x52, 0x73, 0x76, 0xA1, 0x60, 0x62, 0x2E, 0x08, 0x52, 0x08, 0x2E, 0xA9,
        0x60, 0xBC, 0x2E, 0x73, 0x52, 0x0B, 0x0C, 0xBC, 0xEE, 0x0A, 0x2E, 0x08, 0x52, 0x9C, 0x76, 0xA9
    };
    size_t i; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    if (material == NULL || size != sizeof(randNum)) {
        WSEC_LOG_E1("The Rk meterial input is wrong, input length is %zu .", size);
        return;
    }
    for (i = 0; i < size; i++) {
        material[i] = randNum[i];
    }
}

/* Constructs a root key derivation key (RMK) to protect the MK. */
WsecBool KmcMakeRmk(const KmcKsfRk *rk, const WsecBuff *rmkBuff)
{
    unsigned char rkMaterial[KMC_MATERIAL_SIZE];
    unsigned char materialCBuff[KMC_MATERIAL_SIZE] = {0};
    const unsigned char *materialA = NULL;
    const unsigned char *materialB = NULL;
    const unsigned char *materialC = NULL;
    WsecUint32 i;
    unsigned long errorCode; /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    Pbkdf2ParamConst pbkdf2Param;

    if (!(rk != NULL && rmkBuff != NULL)) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_FALSE;
    }
    if (!((rmkBuff->buff != NULL) && (rmkBuff->len > 0))) {
        WSEC_LOG_E2("%s - %d, parameter invalid.", __FUNCTION__, __LINE__);
        return WSEC_FALSE;
    }

    materialA = rk->rkParameters.rkMaterialA;
    materialB = rk->rkParameters.rkMaterialB;
    materialC = materialCBuff;
    GetRkMeterial(materialCBuff, sizeof(materialCBuff));

    for (i = 0; i < sizeof(rkMaterial); i++, materialA++, materialB++, materialC++) {
        rkMaterial[i] = (unsigned char)((*materialA) ^ (*materialB) ^ (*materialC));
    }

    pbkdf2Param.salt = rk->rkParameters.rmkSalt;
    pbkdf2Param.saltLen = (WsecUint32)sizeof(rk->rkParameters.rmkSalt);
    pbkdf2Param.iter = (int)rk->rkAttributes.rmkIter;

    errorCode = CacPbkdf2(WSEC_ALGID_PBKDF2_HMAC_SHA256,
        rkMaterial, (WsecUint32)sizeof(rkMaterial), &pbkdf2Param, rmkBuff->len, rmkBuff->buff);
    (void)memset_s(rkMaterial, sizeof(rkMaterial), 0, sizeof(rkMaterial));
    (void)memset_s(materialCBuff, sizeof(materialCBuff), 0, sizeof(materialCBuff));
    if (errorCode != WSEC_SUCCESS) {
        WSEC_LOG_E1("CacPbkdf2() failed %lu", errorCode);
        return WSEC_FALSE;
    }

    return WSEC_TRUE;
}

/* Protecting the Root Key Material Memory */
unsigned long KmcProtectRkMaterials(KmcKsfRk *rk)
{
    unsigned long ret;
    WSEC_ASSERT(rk != NULL);
    ret = ProtectDataSameBuf(rk->rkParameters.rkMaterialA, (unsigned int)sizeof(rk->rkParameters.rkMaterialA));
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    return ProtectDataSameBuf(rk->rkParameters.rkMaterialB, (unsigned int)sizeof(rk->rkParameters.rkMaterialB));
}

/* Root key material protection is removed. */
unsigned long KmcUnprotectRkMaterials(KmcKsfRk *rk)
{
    unsigned long ret;
    WSEC_ASSERT(rk != NULL);
    ret = UnprotectDataSameBuf(rk->rkParameters.rkMaterialA, (unsigned int)sizeof(rk->rkParameters.rkMaterialA));
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    return UnprotectDataSameBuf(rk->rkParameters.rkMaterialB, (unsigned int)sizeof(rk->rkParameters.rkMaterialB));
}
