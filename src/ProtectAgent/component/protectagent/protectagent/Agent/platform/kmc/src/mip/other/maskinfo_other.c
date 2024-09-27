/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: For other operating systems, use the V2 mask mechanism for protection.
 * Author: z00316590
 * Create: 2019-03-07
 */
#ifdef WSEC_COMPILE_MIP_OTHER
#include "kmcv3_maskinfo.h"
#include "securec.h"
#include "cacv2_pri.h"
#include "wsecv2_util.h"
#include "wsecv2_mem.h"
#include "wsecv2_errorcode.h"

static unsigned char g_maskCode[KMC_MASKCODE_KEY_LENGTH] = {0};
static unsigned char g_xorCheck[KMC_MASKCODE_LENGTH] = {0}; /* MIP is Memory Info Protection */

/* Initialize the mask mechanism. */
unsigned long InitMaskCode(void)
{
    unsigned long ret;
    int i;
    int j;
    ret = CacRandom(g_maskCode, (WsecUint32)sizeof(g_maskCode));
    if (ret != WSEC_SUCCESS) {
        WSEC_LOG_E("Other os get random number failed");
        return WSEC_FAILURE;
    }
    for (i = 0, j = KMC_MASKCODE_LENGTH; i < KMC_MASKCODE_LENGTH; i++, j++) {
        g_xorCheck[i] = g_maskCode[i] ^ g_maskCode[j];
    }
    return WSEC_SUCCESS;
}

/* Other OSs that do not have the memory data protection mechanism use random numbers for protection. */
static unsigned long OtherXorData(const unsigned char *datain, unsigned int inlen,
    unsigned char *dataout, unsigned int *outlen)
{
    unsigned int i;
    unsigned int j;
    unsigned char xorCheck[KMC_MASKCODE_LENGTH];
    if (datain == NULL || dataout == NULL || outlen == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }
    if (*outlen < inlen) {
        return WSEC_ERR_INVALID_ARG;
    }
    for (i = 0, j = KMC_MASKCODE_LENGTH; i < KMC_MASKCODE_LENGTH; i++, j++) {
        xorCheck[i] = g_maskCode[i] ^ g_maskCode[j];
    }
    if (WSEC_MEMCMP(xorCheck, g_xorCheck, KMC_MASKCODE_LENGTH) != 0) {
        WSEC_LOG_E("xor check failed");
        return WSEC_FAILURE;
    }

    *outlen = inlen;
    for (i = 0; i < inlen; i++) {
        j = i % KMC_MASKCODE_LENGTH;
        dataout[i] = (g_maskCode[j] ^ datain[i]);
    }
    return WSEC_SUCCESS;
}

/* Protecting Memory Data */
unsigned long ProtectData(const unsigned char *datain, unsigned int inlen,
    unsigned char *dataout, unsigned int *outlen)
{
    return OtherXorData(datain, inlen, dataout, outlen);
}

/* Disabling Memory Data Protection */
unsigned long UnprotectData(const unsigned char *datain, unsigned int inlen,
    unsigned char *dataout, unsigned int *outlen)
{
    return OtherXorData(datain, inlen, dataout, outlen);
}

/* Protects memory data to the same buffer. */
unsigned long ProtectDataSameBuf(unsigned char *data, unsigned int len)
{
    unsigned int outLen = len;
    unsigned long ret;
    ret = OtherXorData(data, len, data, &outLen);
    (void)outLen;
    return ret;
}

/* Unprotecting the Memory Data to the Same Buffer */
unsigned long UnprotectDataSameBuf(unsigned char *data, unsigned int len)
{
    unsigned int outLen = len;
    unsigned long ret;
    ret = OtherXorData(data, len, data, &outLen);
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
