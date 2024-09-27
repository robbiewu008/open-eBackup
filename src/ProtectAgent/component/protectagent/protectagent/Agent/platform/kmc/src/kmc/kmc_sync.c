/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC - Key Management Component - Sync MK
 * Author: yangdingfu
 * Create: 20-11-03
 * Notes: This File split from kmcv2_ksm.c since the original file is near 2000 lines.
 */

#include "kmc_sync.h"
#include "securec.h"
#include "cacv2_pri.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_mem.h"
#include "wsecv2_order.h"
#include "wsecv2_util.h"
#include "kmc_utils.h"
#include "kmcv2_cfg.h"
#include "kmcv2_ksf.h"
#include "kmcv2_ksm.h"
#include "kmcv3_maskinfo.h"

/* Obtains the data length of the sent master key. */
static unsigned long GetSyncLen(WsecArray mkArray, WsecUint32 mkCount, WsecUint32 *sendLen, WsecUint32 domainId)
{
    WsecUint32 len = KMC_HASH_SHA256_LEN; /* The SHA256 value contains 32 characters. */
    unsigned long ret = WSEC_SUCCESS;
    WsecUint32 i;
    KmcMemMk *memMk = NULL;
    unsigned char domainType;
    for (i = 0; i < mkCount; i++) {
        memMk = (KmcMemMk *)WsecArrGetAt(mkArray, (int)i);
        if (memMk == NULL) {
            continue;
        }

        if (KmcCompareDomain(memMk->mkInfo.domainId, domainId) == KMC_COMPARE_NOT_EQ) {
            continue;
        }

        ret = CfgGetDomainType(memMk->mkInfo.domainId, &domainType);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        if (domainType == KMC_DOMAIN_TYPE_SHARE) {
            len = len + (WsecUint32)sizeof(KmcSyncMk);
        }
    }
    *sendLen = len;
    return ret;
}

/* Add the local domain key to the memory. */
static unsigned long AppendDomainLocal(WsecArray mkArray, int mkCount, KmcKsfMem *ksfMem)
{
    KmcMemMk *memMk = NULL;
    KmcMemMk *local = NULL;
    unsigned char domainType;
    unsigned long ret = WSEC_SUCCESS;
    int i;

    if (mkCount > WSEC_MK_NUM_MAX) {
        WSEC_LOG_E2("mkNum(%d) invalid.(MAX = %u)", mkCount, WSEC_MK_NUM_MAX);
        return WSEC_ERR_KMC_MK_NUM_OVERFLOW;
    }
    for (i = 0; i < mkCount; i++) {
        memMk = (KmcMemMk *)WsecArrGetAt(mkArray, i);
        if (memMk == NULL) {
            continue;
        }
        /*
         * If the domain does not exist, the domain is not a local domain.
         * The local domain requires that the Agent be maintained in the memory.
         * (The value 0 or 1 is a default domain exception. Whether the domain is
         * a local domain will be determined later.)
         * The master must maintain the local domain and all shared domains to the memory.
         */
        ret = CfgGetDomainType(memMk->mkInfo.domainId, &domainType);
        if (ret == WSEC_ERR_KMC_DOMAIN_MISS) {
            ret = WSEC_SUCCESS; /* This situation is reasonable. Therefore, you need to set it to Normal. */
            continue;
        }
        if (ret != WSEC_SUCCESS) {
            break;
        }
        if (domainType != KMC_DOMAIN_TYPE_LOCAL) {
            continue;
        }
        local = (KmcMemMk *)WSEC_CLONE_BUFF(memMk, sizeof(KmcMemMk));
        if (local == NULL) {
            WSEC_LOG_E4MALLOC(sizeof(KmcMemMk));
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }
        ret = AddMkToArray(ksfMem, local, WSEC_TRUE);
        if (ret != WSEC_SUCCESS) {
            WSEC_CLEAR_FREE(local, sizeof(KmcMemMk));
            break;
        }
    }
    return ret;
}

/* Merge mk in domainId from src to dest */
static unsigned long MergeKsfMem(KmcKsfMem *dest, WsecArray baseMkArray, WsecArray recvMkArray, WsecUint32 mergeMode)
{
    unsigned long ret = WSEC_SUCCESS;
    int recvMkCount;
    int kstoreMkCount;
    KmcMemMk *recvMemMk = NULL;
    KmcMemMk *local = NULL;
    int i;
    WSEC_LOG_I("start merge ksf mem");
    recvMkCount = WsecArrGetCount(recvMkArray);
    for (i = 0; i < recvMkCount; i++) {
        recvMemMk = (KmcMemMk *)WsecArrGetAt(recvMkArray, i);
        if (recvMemMk == NULL) {
            continue;
        }

        local = (KmcMemMk *)WSEC_CLONE_BUFF(recvMemMk, sizeof(KmcMemMk));
        if (local == NULL) {
            WSEC_LOG_E4MALLOC(sizeof(KmcMemMk));
            ret = WSEC_ERR_MALLOC_FAIL;
            return ret;
        }

        /* The recveied domain Mks already proctected when receiving, so the third param set to true here */
        ret = AddMkToArray(dest, local, WSEC_TRUE);
        if (ret != WSEC_SUCCESS) {
            WSEC_CLEAR_FREE(local, sizeof(KmcMemMk));
            return ret;
        }
    }

    /* Since has copied all domain mk in step CloneKsfMemWithoutDomain, local Mk needn't append again. */
    if (mergeMode == KMC_ALL_DOMAIN) {
        kstoreMkCount = WsecArrGetCount(baseMkArray);
        ret = AppendDomainLocal(baseMkArray, kstoreMkCount, dest);
    }
    return ret;
}

/* Master send data */
static unsigned long MasterSendSyncData(WsecHandle ctx,
    const unsigned char *buff, WsecUint32 len,
    WsecVoid *param, CallbackSendSyncData sendSyncData)
{
    if (sendSyncData(param, buff, len) == WSEC_FALSE) {
        WSEC_LOG_E1("sendSyncData failed %u", len);
        return WSEC_ERR_KMC_SYNC_MK_FAILED;
    }
    return CacDigestUpdate(ctx, buff, len);
}

static unsigned long MasterSyncSendHeader(WsecHandle ctx, KmcKsfMem *memKeystore, WsecUint32 domainId,
    WsecVoid *param, CallbackSendSyncData sendSyncData)
{
    unsigned long ret;
    WsecUint32 mkCount;
    KmcSyncMkHeader syncHeader;
    syncHeader.msgType = 0;
    syncHeader.version = 1;
    syncHeader.msgLen = 0;
    syncHeader.sharedMkUpdateCounter = memKeystore->sharedMkUpdateCounter;

    mkCount = (WsecUint32)WsecArrGetCount(memKeystore->mkArray);
    ret = GetSyncLen(memKeystore->mkArray, mkCount, &syncHeader.msgLen, domainId);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    WsecCvtByteOrderForSyncMkHeader(&syncHeader, WBCHOST2NETWORK);
    return MasterSendSyncData(ctx, (const unsigned char *)&syncHeader, (WsecUint32)sizeof(KmcSyncMkHeader),
        param, sendSyncData);
}

static unsigned long MasterSyncSendMk(WsecHandle ctx, KmcKsfMem *memKeystore, WsecUint32 domainId,
    WsecVoid *param, CallbackSendSyncData sendSyncData)
{
    unsigned long ret = WSEC_SUCCESS;
    WsecUint32 mkCount;
    KmcMemMk *memMk = NULL;
    KmcSyncMk mk;
    unsigned char domainType;
    int i;

    mkCount = (WsecUint32)WsecArrGetCount(memKeystore->mkArray);
    for (i = 0; i < (int)mkCount; i++) {
        memMk = (KmcMemMk *)WsecArrGetAt(memKeystore->mkArray, i);
        if (memMk == NULL) {
            continue;
        }

        /* only domainId == KMC_ALL_DOMAIN or mkInfo.domainId = domainId allowed */
        if (KmcCompareDomain(memMk->mkInfo.domainId, domainId) == KMC_COMPARE_NOT_EQ) {
            continue;
        }

        ret = CfgGetDomainType(memMk->mkInfo.domainId, &domainType);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        if (domainType != KMC_DOMAIN_TYPE_SHARE) {
            continue;
        }
        (void)memcpy_s(&mk.mkInfo, sizeof(KmcMkInfo), &memMk->mkInfo, sizeof(KmcMkInfo));
        mk.mkRear.plaintextLen = sizeof(mk.mkRear.key);
        ret = UnprotectData(memMk->mkRear.key, memMk->mkRear.plaintextLen, mk.mkRear.key, &mk.mkRear.plaintextLen);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        CvtByteOrderForSyncMk(&mk, WBCHOST2NETWORK);
        ret = MasterSendSyncData(ctx, (const unsigned char *)&mk, (WsecUint32)sizeof(KmcSyncMk), param, sendSyncData);
        if (ret != WSEC_SUCCESS) {
            break;
        }
    }
    (void)memset_s(&mk, sizeof(KmcSyncMk), 0, sizeof(KmcSyncMk));
    return ret;
}

/* Sends all master keys to the Agent. */
static unsigned long MasterSyncMkByDomain(WsecHandle ctx, KmcKsfMem *memKeystore, WsecUint32 domainId,
    WsecVoid *param, CallbackSendSyncData sendSyncData)
{
    unsigned long ret;
    WsecUint32 mkCount;
    mkCount = (WsecUint32)WsecArrGetCount(memKeystore->mkArray);
    if (mkCount > WSEC_MK_NUM_MAX) {
        WSEC_LOG_E2("mkNum(%u) invalid.(MAX = %u)", mkCount, WSEC_MK_NUM_MAX);
        return WSEC_ERR_KMC_MK_NUM_OVERFLOW;
    }

    ret = MasterSyncSendHeader(ctx, memKeystore, domainId, param, sendSyncData);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("Send Mk header error, errno = %lu", ret);
        return ret;
    }

    ret = MasterSyncSendMk(ctx, memKeystore, domainId, param, sendSyncData);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("Send Mk Body error , errno = %lu", ret);
        return ret;
    }
    return ret;
}

/* MASTER: indicates that the AGENT sends all shared master keys. */
unsigned long MemMasterSendMkByDomain(WsecUint32 domainId, WsecVoid *param, CallbackSendSyncData sendSyncData)
{
    unsigned long ret;
    unsigned char hashData[KMC_HASH_SHA256_LEN];
    WsecUint32 hashLen = sizeof(hashData);
    WsecHandle ctx = NULL;
    unsigned long long partHash;
    KmcKsfMem *memKeystore = KsmGetKeystore();

    do {
        if (!IsSupportSyncMk(memKeystore->rk.rkAttributes.version)) {
            WSEC_LOG_E1("[Master]sync must use from Ksf v2 %hu", memKeystore->rk.rkAttributes.version);
            ret = WSEC_ERR_KMC_KSF_VERSION_INVALID;
            break;
        }
        ret = CacDigestInit(&ctx, WSEC_ALGID_SHA256);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = MasterSyncMkByDomain(ctx, memKeystore, domainId, param, sendSyncData);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = CacDigestFinal(&ctx, hashData, &hashLen);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        if (sendSyncData(param, hashData, hashLen) == WSEC_FALSE) {
            WSEC_LOG_E1("sendSyncData failed %u", (WsecUint32)sizeof(KmcSyncMk));
            ret = WSEC_ERR_KMC_SYNC_MK_FAILED;
            break;
        }
        partHash = WsecByteArrToBigInt(hashData, sizeof(hashData));
        WSEC_LOG_I2("Set sync at %llx with counter %u", partHash, memKeystore->updateCounter);
    } while (0);
    CacDigestReleaseCtx(&ctx);
    return ret;
}

/* AGENT receiving */
static unsigned long AgentRecvSyncData(WsecHandle ctx, unsigned char *buff, WsecUint32 len,
    WsecVoid *param, CallbackRecvSyncData recvSyncData)
{
    if (recvSyncData(param, buff, len) == WSEC_FALSE) {
        WSEC_LOG_E1("recvSyncData failed %u", len);
        return WSEC_ERR_KMC_SYNC_MK_FAILED;
    }
    return CacDigestUpdate(ctx, buff, len);
}

/* Receive all shared domain keys from the master. */
static unsigned long AgentSyncMkByDomain(WsecHandle ctx, WsecVoid *param,
    CallbackRecvSyncData recvSyncData, KmcKsfMem *ksfMem)
{
    unsigned long ret;
    KmcSyncMkHeader syncHeader;
    int count;
    int i;
    KmcSyncMk mk;
    ret = AgentRecvSyncData(ctx, (unsigned char *)&syncHeader,
        (WsecUint32)sizeof(KmcSyncMkHeader), param, recvSyncData);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    WsecCvtByteOrderForSyncMkHeader(&syncHeader, WBCNETWORK2HOST);
    if (syncHeader.msgType != 0 || syncHeader.version != 1) {
        WSEC_LOG_E2("type %hu version was %hu", syncHeader.msgType, syncHeader.version);
        return WSEC_ERR_KMC_SYNC_MK_FAILED;
    }

    if (syncHeader.msgLen < KMC_HASH_SHA256_LEN ||
        (syncHeader.msgLen - KMC_HASH_SHA256_LEN) % sizeof(KmcSyncMk) != 0) {
        WSEC_LOG_E1("msgLen %u", syncHeader.msgLen);
        return WSEC_ERR_KMC_SYNC_MK_FAILED;
    }

    count = (int)((syncHeader.msgLen - KMC_HASH_SHA256_LEN) / sizeof(KmcSyncMk));
    if (count > WSEC_MK_NUM_MAX) {
        WSEC_LOG_E1("msgLen %u", syncHeader.msgLen);
        return WSEC_ERR_KMC_SYNC_MK_FAILED;
    }
    for (i = 0; i < count; i++) {   /* soter 573 */
        ret = AgentRecvSyncData(ctx, (unsigned char *)&mk, (WsecUint32)sizeof(KmcSyncMk), param, recvSyncData);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        CvtByteOrderForSyncMk(&mk, WBCNETWORK2HOST);

        ret = CheckDomainIsNotLocalDomain(mk.mkInfo.domainId);
        if (ret != WSEC_SUCCESS) {
            (void)memset_s(&mk, sizeof(KmcSyncMk), 0, sizeof(KmcSyncMk));
            WSEC_LOG_E1("Received mk domain miss or domain type is local domain :%u.", mk.mkInfo.domainId);
            return ret;
        }

        ret = CreateMemMkFromInfoAndPlainKey(&mk.mkInfo, mk.mkRear.key, mk.mkRear.plaintextLen, ksfMem);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("Create Mk from recived mk error %lu.", ret);
            break;
        }
    }
    /* The Agent sharedMkUpdateCounter value is obtained from the synchronization data sent by the Master. */
    ksfMem->sharedMkUpdateCounter = syncHeader.sharedMkUpdateCounter;
    (void)memset_s(&mk, sizeof(KmcSyncMk), 0, sizeof(KmcSyncMk));
    return ret;
}

/* Accept and check the hash value */
static unsigned long AgentRecvAndCheckHash(WsecHandle *ctx,
    WsecVoid *param, CallbackRecvSyncData recvSyncData,
    unsigned char *readHash, WsecUint32 len)
{
    unsigned char hashData[KMC_HASH_SHA256_LEN];
    WsecUint32 hashLen = sizeof(hashData);
    unsigned long ret;
    WSEC_ASSERT(len == KMC_HASH_SHA256_LEN);
    if (recvSyncData(param, readHash, len) == WSEC_FALSE) {
        WSEC_LOG_E1("recvSyncData failed %u", (WsecUint32)sizeof(KmcSyncMkHeader));
        return WSEC_ERR_KMC_SYNC_MK_FAILED;
    }
    ret = CacDigestFinal(ctx, hashData, &hashLen);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }
    if (WSEC_MEMCMP(readHash, hashData, KMC_HASH_SHA256_LEN) != 0) {
        WSEC_LOG_E("because hash not match sync failed");
        return WSEC_ERR_KMC_SYNC_MK_FAILED;
    }
    return ret;
}

static unsigned long AgentAfterRecvMk(KmcKsfMem **targetKsfMem, KmcKsfMem *memKeystore, KmcKsfMem *ksfRecvMem,
    WsecUint32 recvMode, WsecUint32 *outDomainId)
{
    unsigned long ret;
    WsecUint32 uniqueDomainId = KMC_INVALID_DOMAIN;
    switch (recvMode) {
        case KMC_PARTIAL_DOMAIN:
            ret = CheckMkArrayContainUniqueDomain(ksfRecvMem, &uniqueDomainId);
            if (ret != WSEC_SUCCESS) {
                WSEC_LOG_E("Check mk belong to an domain error.");
                return ret;
            }
            ret = CloneKsfMemWithoutDomain(WSEC_TRUE, WSEC_TRUE, memKeystore, targetKsfMem, uniqueDomainId);
            break;
        case KMC_ALL_DOMAIN:
            /* fix uniqueDomainId to KMC_ALL_DOMAIN due to compareMkArr */
            uniqueDomainId = KMC_ALL_DOMAIN;
            ret = CloneKsfMem(WSEC_FALSE, WSEC_TRUE, memKeystore, targetKsfMem);
            break;
        default:
            WSEC_LOG_E1("After recv Mk param check error recvMode %u", recvMode);
            return WSEC_ERR_INVALID_ARG;
    }
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E2("Clone Ksf mem recvMode %u failed %lu", recvMode, ret);
        return ret;
    }
    ret = MergeKsfMem(*targetKsfMem, memKeystore->mkArray, ksfRecvMem->mkArray, recvMode);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E2("Merge Ksf mem recvMode %u failed %lu", recvMode, ret);
        return ret;
    }
    do {
        if (CompareMkArray(ksfRecvMem->mkArray, memKeystore->mkArray, uniqueDomainId) == WSEC_TRUE) {
            WSEC_LOG_I("AgentAfterRecvMk compare mk array is same with g_keystore mkArray.");
            break;
        }
        /*
         * Number of times that the shared MK is updated.
         * Set this parameter to WSEC_FALSE. Set the synchronization data in ksfMem.
         */
        ret = WriteKsfSafety(WSEC_FALSE, NULL, *targetKsfMem, __FUNCTION__);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("WriteKsfSafety() = %lu", ret);
        }
    } while (0);
    if (outDomainId != NULL && ret == WSEC_SUCCESS && uniqueDomainId != KMC_INVALID_DOMAIN) {
        *outDomainId = uniqueDomainId;
    }
    return ret;
}

/* The AGENT receives all shared master keys from the master. */
unsigned long MemAgentRecvMkByDomain(WsecUint32 recvMode,
    WsecUint32 *outDomainId, WsecVoid *param, CallbackRecvSyncData recvSyncData)
{
    unsigned long ret;
    WsecHandle ctx = NULL;
    KmcKsfMem *ksfMem = NULL;
    KmcKsfMem *ksfRecvMem = NULL;
    unsigned char readHash[KMC_HASH_SHA256_LEN];
    unsigned long long partHash;
    KmcKsfMem *memKeystore = KsmGetKeystore();
    do {
        if (!IsSupportSyncMk(memKeystore->rk.rkAttributes.version)) {
            WSEC_LOG_E1("[Agent] sync must use from Ksf v2 %hu", memKeystore->rk.rkAttributes.version);
            ret = WSEC_ERR_KMC_KSF_VERSION_INVALID;
            break;
        }
        ret = CacDigestInit(&ctx, WSEC_ALGID_SHA256);
        if (ret != WSEC_SUCCESS) {
            break;
        }

        ksfRecvMem = (KmcKsfMem *)WSEC_MALLOC(sizeof(KmcKsfMem));
        if (ksfRecvMem == NULL) {
            ret = WSEC_ERR_MALLOC_FAIL;
            break;
        }

        ret = AgentSyncMkByDomain(ctx, param, recvSyncData, ksfRecvMem);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = AgentRecvAndCheckHash(&ctx, param, recvSyncData, readHash, (WsecUint32)sizeof(readHash));
        if (ret != WSEC_SUCCESS) {
            break;
        }

        ret = AgentAfterRecvMk(&ksfMem, memKeystore, ksfRecvMem, recvMode, outDomainId);
        if (ret != WSEC_SUCCESS) {
            break;
        }

        partHash = WsecByteArrToBigInt(readHash, sizeof(readHash));
        WSEC_LOG_I2("Get sync at %llx with counter %u", partHash, ksfMem->updateCounter);
    } while (0);
    /* Switching Keystore Data in the Memory */
    if (ret == WSEC_SUCCESS && ksfMem != NULL) {
        KsmSetKeystore(ksfMem);
    } else {
        (void)FreeKsfSnapshot(ksfMem);
    }
    (void)FreeKsfSnapshot(ksfRecvMem);
    CacDigestReleaseCtx(&ctx);
    return ret;
}
