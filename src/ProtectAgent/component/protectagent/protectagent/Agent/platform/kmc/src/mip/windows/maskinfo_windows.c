/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Windows uses APIs to protect sensitive data in the memory.
 * Author: z00316590
 * Create: 2019-03-07
 */
#ifdef WSEC_COMPILE_MIP_WINDOWS

#include "kmcv3_maskinfo.h"
#include "windows.h"
#include "securec.h"
#include "cacv2_pri.h"
#include "wsecv2_util.h"
#include "wsecv2_errorcode.h"

static unsigned char g_maskCode[KMC_MASKCODE_KEY_LENGTH] = {0};
static unsigned char g_xorCheck[KMC_MASKCODE_LENGTH] = {0}; /* MIP is Memory Info Protection */

/* Initialize the mask mechanism. */
unsigned long InitMaskCode(void)
{
    unsigned long ret;
    int i;
    int j;
    ret = CacRandom(g_maskCode, sizeof(g_maskCode));
    if (ret != WSEC_SUCCESS) {
        return WSEC_FAILURE;
    }
    for (i = 0, j = KMC_MASKCODE_LENGTH; i < KMC_MASKCODE_LENGTH; i++, j++) {
        g_xorCheck[i] = g_maskCode[i] ^ g_maskCode[j];
    }
    if (CryptProtectMemory(g_maskCode, sizeof(g_maskCode), CRYPTPROTECTMEMORY_SAME_PROCESS) == FALSE) {
        WSEC_LOG_E2("ProtectData failed. Code: 0x%X CRYPTPROTECTMEMORY_BLOCK_SIZE=%d\n",
            GetLastError(), CRYPTPROTECTMEMORY_BLOCK_SIZE);
        (void)memset_s(g_maskCode, sizeof(g_maskCode), 0, sizeof(g_maskCode));
        return WSEC_ERR_MEMINFO_PROTECT_FAIL;
    }
    return WSEC_SUCCESS;
}

/* Protecting Memory Data Using APIs in Windows */
static unsigned long WindowsXorData(const unsigned char *datain, unsigned int inlen,
    unsigned char *dataout, unsigned int *outlen)
{
    unsigned int i;
    unsigned int j;
    unsigned char maskCode[KMC_MASKCODE_KEY_LENGTH] = {0};

    if (datain == NULL || dataout == NULL || outlen == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }

    if (*outlen < inlen) {
        return WSEC_ERR_INVALID_ARG;
    }

    if (memcpy_s(maskCode, (size_t)(KMC_MASKCODE_KEY_LENGTH),
        g_maskCode, (size_t)(KMC_MASKCODE_KEY_LENGTH)) != EOK) {
        WSEC_LOG_E4MEMCPY;
        return WSEC_ERR_MEMCPY_FAIL;
    }

    if (FALSE == CryptUnprotectMemory(maskCode, sizeof(maskCode), CRYPTPROTECTMEMORY_SAME_PROCESS)) {
        WSEC_LOG_E1("UProtectData failed. ErrCode: 0x%X\n", GetLastError());
        (void)memset_s(maskCode, sizeof(maskCode), 0, sizeof(maskCode));
        return WSEC_ERR_MEMINFO_PROTECT_FAIL;
    }

    for (i = 0, j = KMC_MASKCODE_LENGTH; i < KMC_MASKCODE_LENGTH; i++, j++) {
        if (g_xorCheck[i] != (maskCode[i] ^ maskCode[j])) {
            WSEC_LOG_E("windows check failed");
            return WSEC_ERR_MEMINFO_PROTECT_FAIL;
        }
    }

    *outlen = inlen;
    for (i = 0; i < inlen; i++) {
        j = i % KMC_MASKCODE_LENGTH;
        dataout[i] = (maskCode[j] ^ datain[i]);
    }
    (void)memset_s(maskCode, sizeof(maskCode), 0, sizeof(maskCode));
    return WSEC_SUCCESS;
}

/* Protecting Memory Data */
unsigned long ProtectData(const unsigned char *datain, unsigned int inlen,
    unsigned char *dataout, unsigned int *outlen)
{
    return WindowsXorData(datain, inlen, dataout, outlen);
}

/* Disabling Memory Data Protection */
unsigned long UnprotectData(const unsigned char *datain, unsigned int inlen,
    unsigned char *dataout, unsigned int *outlen)
{
    return WindowsXorData(datain, inlen, dataout, outlen);
}

/* Protects memory data to the same buffer. */
unsigned long ProtectDataSameBuf(unsigned char *data, unsigned int len)
{
    unsigned int outLen = len;
    unsigned long ret;
    ret = WindowsXorData(data, len, data, &outLen);
    (void)outLen;
    return ret;
}

/* Unprotecting the Memory Data to the Same Buffer */
unsigned long UnprotectDataSameBuf(unsigned char *data, unsigned int len)
{
    unsigned int outLen = len;
    unsigned long ret;
    ret = WindowsXorData(data, len, data, &outLen);
    (void)outLen;
    return ret;
}

/* Deinitialize the mask mechanism. */
void UninitMaskCode(void)
{
    (void)memset_s(g_maskCode, sizeof(g_maskCode), 0, sizeof(g_maskCode));
    (void)memset_s(g_xorCheck, sizeof(g_xorCheck), 0, sizeof(g_xorCheck));
}
#endif
