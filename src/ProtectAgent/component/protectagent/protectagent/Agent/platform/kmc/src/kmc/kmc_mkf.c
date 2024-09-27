/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC - Key Management Component - export & import MKF
 * Author: yangdingfu
 * Create: 20-11-03
 * Notes: This File split from kmcv2_ksm.c since the original file is near 2000 lines.
 */
#include "kmc_mkf.h"
#include "securec.h"
#include "cacv2_pri.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_file.h"
#include "wsecv2_util.h"
#include "wsecv2_mem.h"
#include "wsecv2_order.h"
#include "kmc_utils.h"
#include "kmcv2_ksf.h"
#include "kmcv2_ksm.h"
#include "kmcv3_maskinfo.h"

static const unsigned char g_mkfFlag[KMC_FLAG_LENGTH] = {
    0xF5, 0x06, 0xEF, 0xD8, 0x56, 0x3F, 0xD3, 0x07, 0x3F, 0xDE, 0x29, 0xD7, 0x89, 0xFE, 0xD3, 0xEC,
    0x8D, 0x48, 0xA4, 0x17, 0xC7, 0xDD, 0x3D, 0xD0, 0xF3, 0xED, 0x92, 0x7D, 0x46, 0x3A, 0xAA, 0x0D
};

#define KMC_MAX_CIPHER_LEN_PER_MK           (sizeof(KmcMkFileOneMk) * 2)

/*
 * The export preparation includes the filling header structure KmcMkfHeaderWithHmac.
 * Make sure MemCheckKsfMemAndCfgEx has been checked.
 */
static unsigned long  FillMkfHeader(WsecUint16 mkfVersion, WsecUint32 iter, int mkNum, KmcMkfHeader *header)
{
    unsigned long errorCode = WSEC_SUCCESS;
    unsigned char testKey[KMC_EK4MKF_LEN] = {0};
    KmcMkFileOneMk testPlain;
    /* To prevent ciphertext overflow, multiply the buffer length by 2. */
    unsigned char testCiphertext[KMC_MAX_CIPHER_LEN_PER_MK] = {0};
    WsecUint32 testCipherLen = sizeof(testCiphertext);

    WSEC_ASSERT(header != NULL);
    (void)memset_s(&testPlain, sizeof(testPlain), 0, sizeof(testPlain));
    do {
        /* Fill in each field in the MKF header. */
        header->version = mkfVersion;
        header->ksfVersion = (WsecUint16)(mkfVersion == KMC_MKF_VER_V2 ? KMC_KSF_VER_V2 : KMC_KSF_VER);
        header->cipherAlgId = KMC_ENCRYPT_MK_ALGID;
        header->iterForEncKey = iter;
        if (CacRandom(header->saltForEncKey, (WsecUint32)sizeof(header->saltForEncKey)) != WSEC_SUCCESS) {
            WSEC_LOG_E("CacRandom() fail.");
            errorCode = WSEC_ERR_GET_RAND_FAIL;
            break;
        }
        if (CacRandom(header->ivForEncMk, (WsecUint32)sizeof(header->ivForEncMk)) != WSEC_SUCCESS) {
            WSEC_LOG_E("CacRandom() fail.");
            errorCode = WSEC_ERR_GET_RAND_FAIL;
            break;
        }

        header->hmacAlgId = KMC_HMAC_MK_ALGID;
        header->iterForHmacKey = iter;
        if (CacRandom(header->saltForHmacKey, (WsecUint32)sizeof(header->saltForHmacKey)) != WSEC_SUCCESS) {
            WSEC_LOG_E("CacRandom() fail.");
            errorCode = WSEC_ERR_GET_RAND_FAIL;
            break;
        }

        if (mkNum < 1) {
            WSEC_LOG_E("No MK exist.");
            errorCode = WSEC_ERR_KMC_MK_MISS;
            break;
        }
        header->mkNum = (WsecUint32)mkNum;

        if (CacEncrypt(header->cipherAlgId, testKey, (WsecUint32)sizeof(testKey),
            header->ivForEncMk, (WsecUint32)sizeof(header->ivForEncMk),
            &testPlain, (WsecUint32)sizeof(KmcMkFileOneMk),
            testCiphertext, &testCipherLen) != WSEC_SUCCESS) {
            WSEC_LOG_E("CacEncrypt() fail.");
            errorCode = WSEC_ERR_ENCRPT_FAIL;
            break;
        }
        header->cipherLenPerMk = testCipherLen;
    } while (0);

    return errorCode;
}

/* Derive encKey and hmacKey and write KmcMkfHeaderWithHmac. */
static unsigned long DeriveKeyAndWriteHeaderWithHmac(KmcMkfHeaderWithHmac *headerWithHmac,
    WsecHandle writeFd, WsecBuffConst passwordBuff, WsecBuff encKeyBuff, WsecBuff hmacKeyBuff)
{
    WsecUint32 hmacLen;
    unsigned long ret;
    WsecUint16 mkfVersion;
    WSEC_ASSERT(writeFd != NULL);
    WSEC_ASSERT(headerWithHmac != NULL);
    WSEC_ASSERT(passwordBuff.buff != NULL);
    WSEC_ASSERT(encKeyBuff.buff != NULL);
    WSEC_ASSERT(hmacKeyBuff.buff != NULL);
    mkfVersion = headerWithHmac->mkfHeader.version;
    ret = DeriveKey(headerWithHmac, passwordBuff, encKeyBuff, hmacKeyBuff);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    if (!WSEC_FWRITE_MUST(g_mkfFlag, sizeof(g_mkfFlag), writeFd)) {
        WSEC_LOG_E("Write file fail.");
        return WSEC_ERR_WRI_FILE_FAIL;
    }
    if (mkfVersion == KMC_MKF_VER_V2) {
        CvtByteOrderForMkfHdr(&headerWithHmac->mkfHeader, WBCHOST2NETWORK);
    }
    hmacLen = sizeof(headerWithHmac->hmacData);
    ret = CacHmac(KMC_HMAC_MK_ALGID,
        hmacKeyBuff.buff, hmacKeyBuff.len,
        &headerWithHmac->mkfHeader, (WsecUint32)sizeof(headerWithHmac->mkfHeader),
        headerWithHmac->hmacData, &hmacLen);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("CacHmac() fail.");
        return WSEC_ERR_HMAC_FAIL;
    }
    if (mkfVersion == KMC_MKF_VER) {
        CvtByteOrderForMkfHdr(&headerWithHmac->mkfHeader, WBCHOST2NETWORK);
    }
    if (!WSEC_FWRITE_MUST(headerWithHmac, sizeof(KmcMkfHeaderWithHmac), writeFd)) {
        WSEC_LOG_E("Write file fail.");
        return WSEC_ERR_WRI_FILE_FAIL;
    }
    /*
     * The data in the header information needs to be used during MK encryption.
     * Therefore, the data needs to be restored.
     */
    CvtByteOrderForMkfHdr(&headerWithHmac->mkfHeader, WBCNETWORK2HOST);

    return WSEC_SUCCESS;
}

/* Create an MKF master key from the memory MK. */
static unsigned long MakeMkfMk(const KmcMemMk *memMk, const KmcMkfHeader *header,
    const WsecBuffConst *keyBuff, WsecBuff *mkCipherBuff)
{
    unsigned long ret;
    KmcMkFileOneMk mkWrite;

    (void)memcpy_s(&mkWrite.mkInfo, sizeof(KmcMkInfo), &memMk->mkInfo, sizeof(KmcMkInfo));
    mkWrite.plaintextLen = memMk->mkRear.plaintextLen;

    /* a. Unmask the plaintext masked by the MK to make it true plaintext. */
    ret = UnprotectData(memMk->mkRear.key, memMk->mkRear.plaintextLen, mkWrite.plainText, &mkWrite.plaintextLen);
    if (ret != WSEC_SUCCESS) {
        return ret;
    }

    /* b. Encrypt the MK. */
    ret = CacRandom(mkWrite.iv, (WsecUint32)sizeof(mkWrite.iv)); /* Use random numbers as IVs. */
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("MakeMkfMk CacRandom failed, %lu", ret);
        (void)memset_s(&mkWrite, sizeof(KmcMkFileOneMk), 0, sizeof(KmcMkFileOneMk));
        return ret;
    }
    /* Use random numbers to fill the remaining space of the ciphertext. */
    ret = CacRandom(mkCipherBuff->buff, mkCipherBuff->len);
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("MakeMkfMk CacRandom %lu", ret);
        (void)memset_s(&mkWrite, sizeof(KmcMkFileOneMk), 0, sizeof(KmcMkFileOneMk));
        return ret;
    }
    if (header->version == KMC_MKF_VER_V2) {
        CvtByteOrderForMkfMk(&mkWrite, WBCHOST2NETWORK);
    }
    ret = CacEncrypt(header->cipherAlgId,
        keyBuff->buff, keyBuff->len,
        header->ivForEncMk, (WsecUint32)sizeof(header->ivForEncMk),
        &mkWrite, (WsecUint32)sizeof(KmcMkFileOneMk),
        mkCipherBuff->buff, &mkCipherBuff->len);
    (void)memset_s(&mkWrite, sizeof(KmcMkFileOneMk), 0, sizeof(KmcMkFileOneMk));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E1("CacEncrypt() fail %lu", ret);
        return WSEC_ERR_ENCRPT_FAIL;
    }

    /* The plaintext and ciphertext of the MK must have the same length. Otherwise, a bug occurs. */
    if (mkCipherBuff->len != header->cipherLenPerMk) {
        WSEC_LOG_E2("cipher length different, %u -- %u", header->cipherLenPerMk, mkCipherBuff->len);
        return WSEC_ERR_ENCRPT_FAIL;
    }
    return WSEC_SUCCESS;
}

/* Each MK is encrypted and written into the exported master key file. */
static unsigned long WriteMkToMkf(WsecHandle writeFd, WsecHandle ctx, const KmcMkfHeaderWithHmac *headerWithHmac,
    const unsigned char *encKey, WsecUint32 encKeyLen)
{
    int i;
    KmcMemMk *memMk = NULL;
    /* To prevent ciphertext overflow, multiply the buffer length by 2. */
    unsigned char mkCiphertext[KMC_MAX_CIPHER_LEN_PER_MK] = {0};
    WsecBuff mkCipherBuff;
    WsecBuffConst keyBuff;
    unsigned long ret = WSEC_SUCCESS;
    const KmcMkfHeader *header = NULL;
    KmcKsfMem *memKeystore = KsmGetKeystore();
    WSEC_ASSERT(writeFd != NULL);
    WSEC_ASSERT(ctx != NULL);
    WSEC_ASSERT(headerWithHmac != NULL);
    WSEC_ASSERT(encKey != NULL);

    header = &headerWithHmac->mkfHeader;
    WSEC_BUFF_ASSIGN(keyBuff, encKey, encKeyLen);
    for (i = 0; i < (int)headerWithHmac->mkfHeader.mkNum; i++) {
        memMk = (KmcMemMk *)WsecArrGetAt(memKeystore->mkArray, i);
        if (memMk == NULL) {
            WSEC_LOG_E("memory access fail.");
            ret = WSEC_ERR_OPER_ARRAY_FAIL;
            break;
        }
        WSEC_BUFF_ASSIGN(mkCipherBuff, mkCiphertext, sizeof(mkCiphertext));

        ret = MakeMkfMk(memMk, header, &keyBuff, &mkCipherBuff);
        if (ret != WSEC_SUCCESS) {
            break;
        }

        /* c. Write the MK ciphertext. */
        if (!WSEC_FWRITE_MUST(mkCipherBuff.buff, mkCipherBuff.len, writeFd)) {
            WSEC_LOG_E("Write file fail.");
            ret = WSEC_ERR_WRI_FILE_FAIL;
            break;
        }

        /* d. Calculate HMAC */
        if (CacHmacUpdate(ctx, mkCipherBuff.buff, mkCipherBuff.len) != WSEC_SUCCESS) {
            WSEC_LOG_E("CacHmacUpdate() fail.");
            ret = WSEC_ERR_HMAC_FAIL;
            break;
        }

        if ((i != 0) && (i % (WSEC_EVENT_PERIOD) == 0)) {
            WSEC_DO_EVENTS;
        }
    }

    return ret;
}

/*
 * Each MK is encrypted and written into the exported master key file.
 * The HMAC calculated by all MKs is also written into the exported file.
 */
static unsigned long WriteMkAndHmacToMkf(WsecHandle writeFd,
    const KmcMkfHeaderWithHmac *headerWithHmac,
    WsecBuff encKeyBuff, WsecBuff hmacKeyBuff)
{
    WsecHandle ctx = NULL;
    WsecUint32 hmacAlgId;
    unsigned char hmacForMkCipher[KMC_HMAC_SHA256_LEN] = {0}; /* HMAC of the MK ciphertext */
    WsecUint32 hmacLen;
    unsigned long errorCode;

    do {
        hmacAlgId = headerWithHmac->mkfHeader.hmacAlgId;
        /* Calculate all ciphertext HMACs. */
        if (CacHmacInit(&ctx, hmacAlgId, hmacKeyBuff.buff, hmacKeyBuff.len) != WSEC_SUCCESS) {
            WSEC_LOG_E("CacHmacInit() fail.");
            errorCode = WSEC_ERR_HMAC_FAIL;
            break;
        }

        errorCode = WriteMkToMkf(writeFd, ctx, headerWithHmac, (const unsigned char *)encKeyBuff.buff, encKeyBuff.len);
        if (errorCode != WSEC_SUCCESS) {
            break;
        }

        hmacLen = (WsecUint32)sizeof(hmacForMkCipher);
        if (CacHmacFinal(&ctx, hmacForMkCipher, &hmacLen) != WSEC_SUCCESS) {
            WSEC_LOG_E("CacHmacFinal() fail.");
            errorCode = WSEC_ERR_HMAC_FAIL;
            break;
        }

        /* Write HMAC-related information to the end of the file. */
        if (!WSEC_FWRITE_MUST(hmacForMkCipher, hmacLen, writeFd)) {
            WSEC_LOG_E("Write file fail.");
            errorCode = WSEC_ERR_WRI_FILE_FAIL;
            break;
        }
    } while (0);

    CacHmacReleaseCtx(&ctx);
    return errorCode;
}

/* Encrypt each master key from the password derivation key and export the encrypted master key to a file. */
unsigned long MemExportMkFileEx(WsecUint16 mkfVersion, const char *destFile,
    const unsigned char * const password, const WsecUint32 passwordLen, WsecUint32 iter)
{
    /* If the pclint is initialized, an alarm is reported. (The value 838 is not used.) */
    KmcMkfHeaderWithHmac headerWithHmac;
    unsigned char encKey[KMC_EK4MKF_LEN] = {0}; /* MK encryption key */
    unsigned char hmacKey[KMC_KEY4HMAC_LEN] = {0}; /* HMAC key */
    WsecHandle writeFd = NULL;
    WsecBuffConst passwordBuff;
    WsecBuff encKeyBuff;
    WsecBuff hmacKeyBuff;
    WSEC_BUFF_ASSIGN(passwordBuff, password, passwordLen);
    WSEC_BUFF_ASSIGN(encKeyBuff, encKey, sizeof(encKey));
    WSEC_BUFF_ASSIGN(hmacKeyBuff, hmacKey, sizeof(hmacKey));
    unsigned long ret;
    KmcKsfMem *memKeystore = KsmGetKeystore();
    int mkNum = WsecArrGetCount(memKeystore->mkArray);

    (void)memset_s(&headerWithHmac, sizeof(KmcMkfHeaderWithHmac), 0, sizeof(KmcMkfHeaderWithHmac));

    /* 3. Construct data and write data to a file. */
    do {
        /* 2. Prepare resources, including memory allocation and file opening. */
        writeFd = WSEC_FOPEN(destFile, KMC_FILE_WRITE_BINARY);
        if (writeFd == NULL) {
            WSEC_LOG_E("Open file for write fail.");
            ret = WSEC_ERR_OPEN_FILE_FAIL;
            break;
        }

        /* Filling the MKF header structure */
        ret = FillMkfHeader(mkfVersion, iter, mkNum, &headerWithHmac.mkfHeader);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        /* Derives keys and computes header MAC keys and writes the MKF header structure into a file. */
        ret = DeriveKeyAndWriteHeaderWithHmac(&headerWithHmac, writeFd, passwordBuff, encKeyBuff, hmacKeyBuff);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        /*
         * Encrypt each memory MK and write the ciphertext into the export file.
         * Each ciphertext is considered as HMAC. Then, write HMAC into the export file.
         */
        ret = WriteMkAndHmacToMkf(writeFd, &headerWithHmac, encKeyBuff, hmacKeyBuff);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        WSEC_LOG_I("Export MK succeed");
    } while (0);

    (void)memset_s(encKey, sizeof(encKey), 0, sizeof(encKey));
    (void)memset_s(hmacKey, sizeof(hmacKey), 0, sizeof(hmacKey));
    if (WSEC_FFLUSH(writeFd) != 0) {
        WSEC_LOG_E2("MemExportMkFileEx WSEC_FFLUSH failed, errno %d err %lu", WSEC_FERRNO(writeFd), ret);
        ret = WSEC_ERR_FILE_FLUSH_FAIL;
    }
    WSEC_FCLOSE(writeFd);
    return ret;
}

/* Read KmcMkfHeaderWithHmac to derive encKey and hmacKey. */
static unsigned long ReadHeaderWithHmacAndDeriveKey(WsecHandle readFd, WsecBuffConst passwordBuff,
    KmcMkfHeaderWithHmac *headerWithHmac, WsecBuff encKeyBuff, WsecBuff hmacKeyBuff)
{
    unsigned char formatFlag[KMC_FLAG_LENGTH]; /* MK file format ID. */
    unsigned long errorCode;
    KmcMkfHeader headerNetwork;
    unsigned char hmacData[WSEC_HMAC_LEN_MAX] = {0};
    WsecUint32 hmacLen = sizeof(hmacData);
    KmcMkfHeader *header = NULL;

    WSEC_ASSERT(readFd != NULL);
    WSEC_ASSERT(passwordBuff.buff != NULL);
    WSEC_ASSERT(headerWithHmac != NULL);
    WSEC_ASSERT(encKeyBuff.buff != NULL);
    WSEC_ASSERT(hmacKeyBuff.buff != NULL);
    if (!WSEC_FREAD_MUST(formatFlag, sizeof(formatFlag), readFd)) {
        WSEC_LOG_E("Read mkfFlag fail.");
        return WSEC_ERR_READ_FILE_FAIL;
    }
    if (WSEC_MEMCMP(formatFlag, g_mkfFlag, sizeof(g_mkfFlag)) != 0) {
        WSEC_LOG_E("The file is not MK file.");
        return WSEC_ERR_FILE_FORMAT;
    }

    if (!WSEC_FREAD_MUST(headerWithHmac, sizeof(KmcMkfHeaderWithHmac), readFd)) {
        WSEC_LOG_E("Read KmcMkfHeaderWithHmac fail.");
        return WSEC_ERR_READ_FILE_FAIL;
    }
    /*
     * The network byte order is saved because the network order is verified in V2 and the
     * host order is verified in V1.
     */
    (void)memcpy_s(&headerNetwork, sizeof(KmcMkfHeader), &headerWithHmac->mkfHeader, sizeof(KmcMkfHeader));

    header = &headerWithHmac->mkfHeader;
    CvtByteOrderForMkfHdr(header, WBCNETWORK2HOST); /* Reading and Using Some Parameters */
    if (((header->cipherLenPerMk > KMC_MAX_CIPHER_LEN_PER_MK) ||
        (header->cipherLenPerMk < sizeof(KmcMkFileOneMk)) ||
        (header->version != KMC_MKF_VER_V2 && header->version != KMC_MKF_VER))) {
        WSEC_LOG_E1("%u is not correct MK stru length.", header->cipherLenPerMk);
        return WSEC_ERR_FILE_FORMAT;
    }

    errorCode = DeriveKey(headerWithHmac, passwordBuff, encKeyBuff, hmacKeyBuff);
    if (errorCode != WSEC_SUCCESS) {
        return errorCode;
    }

    /* Calculates the HMAC for the file header. */
    if ((CacHmac(header->hmacAlgId,
        hmacKeyBuff.buff, hmacKeyBuff.len,
        (header->version == KMC_MKF_VER_V2 ? &headerNetwork : header), (WsecUint32)sizeof(KmcMkfHeader),
        hmacData, &hmacLen) != WSEC_SUCCESS)) {
        WSEC_LOG_E("CacHmac() fail.");
        return WSEC_ERR_HMAC_FAIL;
    }

    /*
     * Check whether the HMAC verification file header is damaged. If the verification is successful,
     * the file header is not modified. In this case, KmcMkfHeaderWithHmac indicates that the exported
     * data is not tampered with.
     */
    if (WSEC_MEMCMP(hmacData, headerWithHmac->hmacData, sizeof(headerWithHmac->hmacData)) != 0) {
        WSEC_LOG_E("The file is tampered.");
        return WSEC_ERR_HMAC_FAIL;
    }

    return WSEC_SUCCESS;
}

/* Each MK is decrypted and then read into the memory. */
static unsigned long ReadMkFromMkf(WsecHandle readFd, WsecHandle ctx, const KmcMkfHeaderWithHmac *headerWithHmac,
    const WsecBuff encKeyBuff, KmcKsfMem *ksfMem)
{
    const KmcMkfHeader *header = NULL;
    unsigned char ciphertext[KMC_MAX_CIPHER_LEN_PER_MK] = {0};
    int i;
    unsigned long ret = WSEC_SUCCESS;
    KmcMkFileOneMk mkPlain;
    WsecUint32 plainLen = (WsecUint32)sizeof(mkPlain);

    WSEC_ASSERT(readFd != NULL);
    WSEC_ASSERT(ctx != NULL);
    WSEC_ASSERT(headerWithHmac != NULL);
    WSEC_ASSERT(encKeyBuff.buff != NULL);
    WSEC_ASSERT(ksfMem != NULL);

    header = &headerWithHmac->mkfHeader;
    if (header->cipherLenPerMk > sizeof(ciphertext)) {
        return WSEC_ERR_OUTPUT_BUFF_NOT_ENOUGH;
    }

    /* Read the MK ciphertext one by one. */
    for (i = 0; i < (int)header->mkNum; i++) {
        if (!WSEC_FREAD_MUST(ciphertext, header->cipherLenPerMk, readFd)) {
            WSEC_LOG_E("Read data from file fail.");
            ret = WSEC_ERR_READ_FILE_FAIL;
            break;
        }

        /* Calculates HMAC for the current ciphertext. */
        if (CacHmacUpdate(ctx, ciphertext, header->cipherLenPerMk) != WSEC_SUCCESS) {
            WSEC_LOG_E("CacHmacUpdate() fail.");
            ret = WSEC_ERR_HMAC_FAIL;
            break;
        }

        /* Decrypt the MK. */
        ret = CacDecrypt(header->cipherAlgId,
            encKeyBuff.buff, encKeyBuff.len, header->ivForEncMk, (WsecUint32)sizeof(header->ivForEncMk),
            ciphertext, header->cipherLenPerMk, &mkPlain, &plainLen);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("CacDecrypt fail %lu", ret);
            ret = WSEC_ERR_DECRPT_FAIL;
            break;
        }

        if (header->version == KMC_MKF_VER_V2) {
            CvtByteOrderForMkfMk(&mkPlain, WBCNETWORK2HOST);
        }
        ret = CreateMemMkFromInfoAndPlainKey(&mkPlain.mkInfo, mkPlain.plainText, mkPlain.plaintextLen, ksfMem);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("CreateMemMkFromInfoAndPlainKey() = %lu", ret);
            break;
        }
        if ((i != 0) && (i % (WSEC_EVENT_PERIOD) == 0)) {
            WSEC_DO_EVENTS;
        }
    }
    (void)memset_s(&mkPlain, sizeof(KmcMkFileOneMk), 0, sizeof(KmcMkFileOneMk));

    return ret;
}

/*
 * Each MK is encrypted and written into the exported master key file.
 * The HMAC calculated by all MKs is also written into the exported file.
 */
static unsigned long ReadMkAndVerifyHmac(WsecHandle readFd,
    const KmcMkfHeaderWithHmac *headerWithHmac,
    WsecBuff encKeyBuff,
    WsecBuff hmacKeyBuff,
    KmcKsfMem *ksfMem)
{
    const KmcMkfHeader *header = NULL;
    WsecHandle ctx = NULL;
    unsigned char hmacData[WSEC_HMAC_LEN_MAX] = {0};
    WsecUint32 hmacLen;
    unsigned long errorCode;
    unsigned char hmacForMkCipher[KMC_HMAC_SHA256_LEN] = {0}; /* HMAC of the MK ciphertext */

    WSEC_ASSERT(readFd != NULL);
    WSEC_ASSERT(headerWithHmac != NULL);
    WSEC_ASSERT(encKeyBuff.buff != NULL);
    WSEC_ASSERT(hmacKeyBuff.buff != NULL);
    WSEC_ASSERT(ksfMem != NULL);

    header = &headerWithHmac->mkfHeader;
    do {
        if (CacHmacInit(&ctx, header->hmacAlgId, hmacKeyBuff.buff, hmacKeyBuff.len) != WSEC_SUCCESS) {
            WSEC_LOG_E("CacHmacInit() fail.");
            errorCode = WSEC_ERR_HMAC_FAIL;
            break;
        }

        errorCode = ReadMkFromMkf(readFd, ctx, headerWithHmac, encKeyBuff, ksfMem);
        if (errorCode != WSEC_SUCCESS) {
            break;
        }
        /* Obtain the HMAC of the MK ciphertext and verify the integrity. */
        hmacLen = (WsecUint32)sizeof(hmacData);
        if (CacHmacFinal(&ctx, hmacData, &hmacLen) != WSEC_SUCCESS) {
            WSEC_LOG_E("CacHmacFinal() fail.");
            errorCode = WSEC_ERR_HMAC_FAIL;
            break;
        }

        /* Read the HMACs of all MK ciphertexts in the file. */
        if (!WSEC_FREAD_MUST(hmacForMkCipher, (WsecUint32)sizeof(hmacForMkCipher), readFd)) {
            WSEC_LOG_E("Read data from file fail.");
            errorCode = WSEC_ERR_READ_FILE_FAIL;
            break;
        }

        /*
         * Read 32 bytes. HamcData is 64 bytes. The recalculation result is hmacLen, which is not considered.
         * Only 32 bytes are compared because SHA256 is used for encryption.
         */
        if (WSEC_MEMCMP(hmacData, hmacForMkCipher, sizeof(hmacForMkCipher)) != 0) {
            WSEC_LOG_E("The file's HMAC authenticated fail.");
            errorCode = WSEC_ERR_HMAC_AUTH_FAIL;
        }
    } while (0);
    CacHmacReleaseCtx(&ctx);
    return errorCode;
}

/* Read the MKs from the MKF file one by one and create the g_keystore memory data and KSF based on the read result. */
static unsigned long CreateKsfFromMkf(WsecHandle readFd,
    const KmcMkfHeaderWithHmac *headerWithHmac,
    WsecBuff encKeyBuff,
    WsecBuff hmacKeyBuff)
{
    unsigned long ret;
    /* After the function is invoked, it may be stored in g_keystore. The heap memory must be used. */
    KmcKsfMem *ksfMem = NULL;
    WsecBool shareDomainMkChanged = WSEC_FALSE;
    KmcKsfMem *memKeystore = KsmGetKeystore();
    WSEC_ASSERT(readFd != NULL);
    WSEC_ASSERT(headerWithHmac != NULL);
    WSEC_ASSERT(encKeyBuff.buff != NULL);
    WSEC_ASSERT(hmacKeyBuff.buff != NULL);
    /* In V3 and later versions, the imported MK root key remains unchanged. */
    do {
        ret = CloneKsfMem(WSEC_FALSE, WSEC_TRUE, memKeystore, &ksfMem);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = ReadMkAndVerifyHmac(readFd, headerWithHmac, encKeyBuff, hmacKeyBuff, ksfMem);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        /*
         * Write the new Keystore to a file, switch the Keystore data in the memory,
         * and increase the number of times that the Master shared MK is updated (new).
         * The number of times that the shared MK of the Agent is updated remains unchanged.
         * After the MK is imported to the Master, the MK needs to be synchronized.
         */
        if (PriKmcSysGetRole() == KMC_ROLE_MASTER) {
            shareDomainMkChanged = WSEC_TRUE;
        }
        ret = WriteKsfSafety(shareDomainMkChanged, NULL, ksfMem, __FUNCTION__);
        if (ret != WSEC_SUCCESS) {
            WSEC_LOG_E1("WriteKsfSafety() = %lu.", ret);
        }
    } while (0);
    /* Switching Keystore Data in the Memory */
    if (ret == WSEC_SUCCESS && ksfMem != NULL) {
        KsmSetKeystore(ksfMem);
    } else {
        (void)FreeKsfSnapshot(ksfMem);
    }
    return ret;
}

/* Importing All CMKs from a CMK Export File */
unsigned long MemImportMkFileEx(const char *fromFile, const unsigned char * const password,
    const WsecUint32 passwordLen)
{
    KmcMkfHeaderWithHmac headerWithHmac;
    WsecHandle readFd = NULL;
    unsigned long ret;
    unsigned char encKey[KMC_EK4MKF_LEN] = {0};
    unsigned char hmacKey[KMC_KEY4HMAC_LEN] = {0};
    WsecBuffConst passwordBuff;
    WsecBuff encKeyBuff;
    WsecBuff hmacKeyBuff;
    WSEC_BUFF_ASSIGN(passwordBuff, password, passwordLen);
    WSEC_BUFF_ASSIGN(encKeyBuff, encKey, sizeof(encKey));
    WSEC_BUFF_ASSIGN(hmacKeyBuff, hmacKey, sizeof(hmacKey));

    /* Reads the file header and determines the file format. */
    do {
        readFd = WSEC_FOPEN(fromFile, KMC_FILE_READ_BINARY);
        if (readFd == NULL) {
            WSEC_LOG_E("Open file for read fail.");
            ret = WSEC_ERR_OPEN_FILE_FAIL;
            break;
        }

        ret = ReadHeaderWithHmacAndDeriveKey(readFd, passwordBuff, &headerWithHmac, encKeyBuff, hmacKeyBuff);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        ret = CreateKsfFromMkf(readFd, &headerWithHmac, encKeyBuff, hmacKeyBuff);
        if (ret != WSEC_SUCCESS) {
            break;
        }
        WSEC_LOG_I("Import MK  succeed.");
    } while (0);

    /* Resource release */
    WSEC_FCLOSE(readFd);
    (void)memset_s(encKey, sizeof(encKey), 0, sizeof(encKey));
    (void)memset_s(hmacKey, sizeof(hmacKey), 0, sizeof(hmacKey));
    return ret;
}
