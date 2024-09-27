/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC internal interfaces are not open to external systems.
 * Author: x00102361
 * Create: 2014-06-16
 */

#ifndef KMC_SRC_COMMON_WSECV2_UTIL_H
#define KMC_SRC_COMMON_WSECV2_UTIL_H

#include "wsecv2_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 1.2 Either iPSI or OpenSSL or mbedtls can be specified. */
#ifdef WSEC_COMPILE_CAC_IPSI
#if defined(WSEC_COMPILE_CAC_OPENSSL) || defined(WSEC_COMPILE_CAC_MBEDTLS)
#error Cannot defined both 'WSEC_COMPILE_CAC_IPSI' and '(WSEC_COMPILE_CAC_OPENSSL' or 'WSEC_COMPILE_CAC_MBEDTLS)'
#endif
#endif

#ifdef WSEC_COMPILE_CAC_OPENSSL
#if defined(WSEC_COMPILE_CAC_MBEDTLS) || defined(WSEC_COMPILE_CAC_IPSI)
#error Cannot defined both 'WSEC_COMPILE_CAC_OPENSSL' and 'WSEC_COMPILE_CAC_MBEDTLS' or 'WSEC_COMPILE_CAC_IPSI'
#endif
#endif

#ifdef WSEC_COMPILE_CAC_MBEDTLS
#if defined(WSEC_COMPILE_CAC_IPSI) || defined(WSEC_COMPILE_CAC_OPENSSL)
#error Cannot defined both 'WSEC_COMPILE_CAC_MBEDTLS' and 'WSEC_COMPILE_CAC_IPSI' or 'WSEC_COMPILE_CAC_OPENSSL'
#endif
#endif

/* 2. Macro Definition */
/* 2.1 Constant Macro */
#define WSEC_LOG_BUFF_SIZE    512 /* Maximum length of a log. */

/* 2.3 Log Function Macro */
#define WSEC_LOG(level, logText) WsecLog(WSEC_KMC_FILE, __LINE__, level, "%s", logText)
#define WSEC_LOG1(level, fmt, v1) WsecLog(WSEC_KMC_FILE, __LINE__, level, fmt, v1)
#define WSEC_LOG2(level, fmt, v1, v2) WsecLog(WSEC_KMC_FILE, __LINE__, level, fmt, v1, v2)
#define WSEC_LOG3(level, fmt, v1, v2, v3) WsecLog(WSEC_KMC_FILE, __LINE__, level, fmt, v1, v2, v3)
#define WSEC_LOG4(level, fmt, v1, v2, v3, v4) WsecLog(WSEC_KMC_FILE, __LINE__, level, fmt, v1, v2, v3, v4)
#define WSEC_LOG5(level, fmt, v1, v2, v3, v4, v5) WsecLog(WSEC_KMC_FILE, __LINE__, level, fmt, v1, v2, v3, v4, v5)

/* 1) Error logs */
#define WSEC_LOG_E(logText) WSEC_LOG(WSEC_LOG_ERR, logText)
#define WSEC_LOG_E1(fmt, v1) WSEC_LOG1(WSEC_LOG_ERR, fmt, v1)
#define WSEC_LOG_E2(fmt, v1, v2) WSEC_LOG2(WSEC_LOG_ERR, fmt, v1, v2)
#define WSEC_LOG_E3(fmt, v1, v2, v3) WSEC_LOG3(WSEC_LOG_ERR, fmt, v1, v2, v3)
#define WSEC_LOG_E4(fmt, v1, v2, v3, v4) WSEC_LOG4(WSEC_LOG_ERR, fmt, v1, v2, v3, v4)
#define WSEC_LOG_E5(fmt, v1, v2, v3, v4, v5) WSEC_LOG5(WSEC_LOG_ERR, fmt, v1, v2, v3, v4, v5)

/* 2) Warning logs */
#define WSEC_LOG_W(logText) WSEC_LOG(WSEC_LOG_WARN, logText)
#define WSEC_LOG_W1(fmt, v1) WSEC_LOG1(WSEC_LOG_WARN, fmt, v1)
#define WSEC_LOG_W2(fmt, v1, v2) WSEC_LOG2(WSEC_LOG_WARN, fmt, v1, v2)
#define WSEC_LOG_W3(fmt, v1, v2, v3) WSEC_LOG3(WSEC_LOG_WARN, fmt, v1, v2, v3)
#define WSEC_LOG_W4(fmt, v1, v2, v3, v4) WSEC_LOG4(WSEC_LOG_WARN, fmt, v1, v2, v3, v4)
#define WSEC_LOG_W5(fmt, v1, v2, v3, v4, v5) WSEC_LOG5(WSEC_LOG_WARN, fmt, v1, v2, v3, v4, v5)

/* 3) Warning logs */
#define WSEC_LOG_I(logText) WSEC_LOG(WSEC_LOG_INFO, logText)
#define WSEC_LOG_I1(fmt, v1) WSEC_LOG1(WSEC_LOG_INFO, fmt, v1)
#define WSEC_LOG_I2(fmt, v1, v2) WSEC_LOG2(WSEC_LOG_INFO, fmt, v1, v2)
#define WSEC_LOG_I3(fmt, v1, v2, v3) WSEC_LOG3(WSEC_LOG_INFO, fmt, v1, v2, v3)
#define WSEC_LOG_I4(fmt, v1, v2, v3, v4) WSEC_LOG4(WSEC_LOG_INFO, fmt, v1, v2, v3, v4)
#define WSEC_LOG_I5(fmt, v1, v2, v3, v4, v5) WSEC_LOG5(WSEC_LOG_INFO, fmt, v1, v2, v3, v4, v5)

/* 4) Operation failure logs */
#define WSEC_LOG_E4MALLOC(memSize) \
    WSEC_LOG_E1("Allocate Memory(size=%u) fail.", ((WsecUint32)(memSize))) /* Failed to allocate memory. */

#define WSEC_LOG_E4MEMCPY WSEC_LOG_E("copy memory fail.") /* Failed to copy the memory. */
#define WSEC_LOG_E4MEMSET WSEC_LOG_E("reset memory fail.") /* Failed to set the memory. */

/* 5. Debug trace */
#ifdef WSEC_DEBUG
    #define WSEC_TRACE(traceText) WSEC_LOG_I(traceText)
    #define WSEC_TRACE1(fmt, v1) WSEC_LOG_I1(fmt, v1)
    #define WSEC_TRACE2(fmt, v1, v2) WSEC_LOG_I2(fmt, v1, v2)
    #define WSEC_TRACE3(fmt, v1, v2, v3) WSEC_LOG_I3(fmt, v1, v2, v3)
    #define WSEC_TRACE4(fmt, v1, v2, v3, v4) WSEC_LOG_I4(fmt, v1, v2, v3, v4)
    #define WSEC_TRACE5(fmt, v1, v2, v3, v4, v5) WSEC_LOG_I5(fmt, v1, v2, v3, v4, v5)
#else
    #define WSEC_TRACE(traceText)
    #define WSEC_TRACE1(fmt, v1)
    #define WSEC_TRACE2(fmt, v1, v2)
    #define WSEC_TRACE3(fmt, v1, v2, v3)
    #define WSEC_TRACE4(fmt, v1, v2, v3, v4)
    #define WSEC_TRACE5(fmt, v1, v2, v3, v4, v5)
#endif

/* 6) Comparison result */
#define WSEC_CMP_RST_SMALL_THAN (-1)
#define WSEC_CMP_RST_EQUAL      0
#define WSEC_CMP_RST_BIG_THAN   1

#define WSEC_EVENT_PERIOD 400

/* 3. Enumeration */
/* Log Level */
typedef enum {
    WSEC_LOG_INFO,
    WSEC_LOG_WARN,
    WSEC_LOG_ERR
} WsecLogLevel;

/* Print logs. */
WsecVoid WsecLog(const char *file, int line, int level, const char *fmt, ...);

/* byte array converted to unsigned long long number */
unsigned long long WsecByteArrToBigInt(const unsigned char *byteArr, size_t len);
/* Encryption (software-layer root key or master key) */
unsigned long WsecKmcHwEncData(WsecHandle handle, const unsigned char *plaintext, unsigned int plaintextLen,
    unsigned char *ciphertext, unsigned int *ciphertextLen);

/* Decryption (software-layer root key or master key) */
unsigned long WsecKmcHwDecData(WsecHandle handle, const unsigned char *ciphertext, unsigned int ciphertextLen,
    unsigned char *plaintext, unsigned int *plaintextLen);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* KMC_SRC_COMMON_WSECV2_UTIL_H */
