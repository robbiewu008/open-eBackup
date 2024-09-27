/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Linux uses keyring to protect sensitive information in the memory.
 * Author: z00316590
 * Create: 2019-03-07
 */
#ifdef WSEC_COMPILE_MIP_LINUX
#include "kmcv3_maskinfo.h"
#include <linux/keyctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "securec.h"
#include "cacv2_pri.h"
#include "kmcv3_keyring.h"
#include "wsecv2_errorcode.h"
#include "wsecv2_util.h"
#include "wsecv2_mem.h"

static unsigned char g_xorCheck[KMC_MASKCODE_LENGTH] = {0}; /* MIP is Memory Info Protection */

#define KMC_MASKINFO_SET_KEY_NAME_RADIX 10
static char g_keyName[48] = {0}; /* The value is a string of up to 48 digits. */
static WsecBool g_hasName = WSEC_FALSE;

/* Initialize keyName. */
static void KeyringSetKeyName(void)
{
    if (g_hasName == WSEC_FALSE) {
        unsigned long pid = (unsigned long)(long)getpid();
        const char num[KMC_MASKINFO_SET_KEY_NAME_RADIX] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
        size_t idx = strlen("kmcv3maskcode");

        (void)strcpy_s(g_keyName, sizeof(g_keyName), "kmcv3maskcode");
        /* The index is calculated and does not exceed the threshold. */
        while (pid > 0) {
            g_keyName[idx] = num[pid % KMC_MASKINFO_SET_KEY_NAME_RADIX];
            pid = pid / KMC_MASKINFO_SET_KEY_NAME_RADIX;
            idx++;
        }
        g_keyName[idx] = '\0';
        g_hasName = WSEC_TRUE;
    }
}

/* Removing a Kernel Key */
static void RemoveMaskCodeKey(void)
{
    long key;
    long ret;
    unsigned char zeroBuff[KMC_MASKCODE_KEY_LENGTH] = {0};

    KeyringSetKeyName();
    key = KmcKeyringRequestKey("user", g_keyName, NULL, (long)KEY_SPEC_SESSION_KEYRING);
    if (key == -1) {
        WSEC_LOG_I1("Linux keyring request no key, errno=%d", errno);
        return;
    }
    ret = KmcKeyringUpdate(key, zeroBuff, sizeof(zeroBuff));
    if (ret == -1) {
        WSEC_LOG_I1("Linux keyring update key failed, errno=%d", errno);
    }
    ret = KmcKeyringRevoke(key);
    if (ret == -1) {
        WSEC_LOG_I1("Linux keyring revoke key failed, errno=%d", errno);
    }
    WSEC_LOG_I("Linux keyring remove key success\n");
    (void)memset_s(g_xorCheck, sizeof(g_xorCheck), 0, sizeof(g_xorCheck));
}

/* Initialize the mask mechanism. */
unsigned long InitMaskCode(void)
{
    unsigned char maskCode[KMC_MASKCODE_KEY_LENGTH];
    long key;
    int i;
    int j;
    long ret;
    KeyringSetKeyName();
    if (CacRandom(maskCode, (WsecUint32)sizeof(maskCode)) != WSEC_SUCCESS) {
        WSEC_LOG_E("Linux keyring get random number failed");
        return WSEC_FAILURE;
    }
    RemoveMaskCodeKey();
    key = KmcKeyringAddKey("user", g_keyName, maskCode, sizeof(maskCode), (long)KEY_SPEC_SESSION_KEYRING);
    if (key == -1) {
        WSEC_LOG_E1("Linux keyring add key failed, errno=%d", errno);
        (void)memset_s(maskCode, sizeof(maskCode), 0, sizeof(maskCode));
        return WSEC_FAILURE;
    }
    ret = KmcKeyringSetTimeOut(key, (long)0);
    if (ret == -1) {
        WSEC_LOG_E1("Linux keyring set time out failed, errno=%d", errno);
        (void)memset_s(maskCode, sizeof(maskCode), 0, sizeof(maskCode));
        return WSEC_FAILURE;
    }
    for (i = 0, j = KMC_MASKCODE_LENGTH; i < KMC_MASKCODE_LENGTH; i++, j++) {
        g_xorCheck[i] = maskCode[j] ^ maskCode[i];
    }
    (void)memset_s(maskCode, sizeof(maskCode), 0, sizeof(maskCode));
    WSEC_LOG_I("Maskcode init successfully");
    return WSEC_SUCCESS;
}

/* The Linux OS keying mechanism protects the memory. */
static unsigned long LinuxXorData(const unsigned char *datain, unsigned int inlen,
    unsigned char *dataout, unsigned int *outlen)
{
    long key;
    long ret;
    unsigned int i;
    unsigned int j;
    unsigned char maskCode[KMC_MASKCODE_KEY_LENGTH];
    unsigned char xorCheck[KMC_MASKCODE_LENGTH];
    if (datain == NULL || dataout == NULL || outlen == NULL) {
        return WSEC_ERR_INVALID_ARG;
    }
    if (*outlen < inlen) {
        return WSEC_ERR_INVALID_ARG;
    }
    KeyringSetKeyName();
    key = KmcKeyringRequestKey("user", g_keyName, NULL, (long)KEY_SPEC_SESSION_KEYRING);
    if (key == -1) {
        WSEC_LOG_E1("Linux keyring request key failed, errno=%d", errno);
        return WSEC_FAILURE;
    }
    ret = KmcKeyringReadKey(key, maskCode, sizeof(maskCode));
    if (ret == -1) {
        WSEC_LOG_E1("Linux keyring read key failed , errno=%d", errno);
        (void)memset_s(maskCode, sizeof(maskCode), 0x00, sizeof(maskCode));
        return WSEC_FAILURE;
    }
    for (i = 0, j = KMC_MASKCODE_LENGTH; i < KMC_MASKCODE_LENGTH; i++, j++) {
        xorCheck[i] = maskCode[j] ^ maskCode[i];
    }
    if (WSEC_MEMCMP(xorCheck, g_xorCheck, KMC_MASKCODE_LENGTH) != 0) {
        WSEC_LOG_E("Linux keyring key is not right");
        (void)memset_s(maskCode, sizeof(maskCode), 0x00, sizeof(maskCode));
        return WSEC_FAILURE;
    }
    *outlen = inlen;
    for (i = 0; i < inlen; i++) {
        j = i % KMC_MASKCODE_LENGTH;
        dataout[i] = (maskCode[j] ^ datain[i]);
    }
    (void)memset_s(maskCode, sizeof(maskCode), 0x00, sizeof(maskCode));
    (void)memset_s(xorCheck, sizeof(xorCheck), 0, sizeof(xorCheck));
    return WSEC_SUCCESS;
}

/* Protecting Memory Data */
unsigned long ProtectData(const unsigned char *datain, unsigned int inlen,
    unsigned char *dataout, unsigned int *outlen)
{
    return LinuxXorData(datain, inlen, dataout, outlen);
}

/* Disabling Memory Data Protection */
unsigned long UnprotectData(const unsigned char *datain, unsigned int inlen,
    unsigned char *dataout, unsigned int *outlen)
{
    return LinuxXorData(datain, inlen, dataout, outlen);
}

/* Protects memory data to the same buffer. */
unsigned long ProtectDataSameBuf(unsigned char *data, unsigned int len)
{
    unsigned int outLen = len;
    unsigned long ret;
    ret = LinuxXorData(data, len, data, &outLen);
    (void)outLen;
    return ret;
}

/* Unprotecting the Memory Data to the Same Buffer */
unsigned long UnprotectDataSameBuf(unsigned char *data, unsigned int len)
{
    unsigned int outLen = len;
    unsigned long ret;
    ret = LinuxXorData(data, len, data, &outLen);
    (void)outLen;
    return ret;
}

/* Deinitialize the mask mechanism. */
void UninitMaskCode(void)
{
    RemoveMaskCodeKey();
}
#endif
