/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: header file of SDP V2 external interfaces
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#ifndef KMC_SRC_SDP_SDPV2_ITF_H
#define KMC_SRC_SDP_SDPV2_ITF_H

#include "wsecv2_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Key derivation length */
#define SDP_HMAC_DERIVE_KEY_LEN 16u

/* Structure */
/* Ciphertext Header Definition */
#pragma pack(1)
typedef struct {
    WsecUint32    cipherVersion;
    WsecUint32    hmacFlag;    /* Determine whether to add HMAC */
    WsecUint32    domainId;
    WsecUint32    keyId;
    WsecUint32    cipherAlgId;
    unsigned char mkHashId[WSEC_MK_HASH_REC_LEN]; /* The first eight bytes of the MK SHA256. */
    unsigned char salt[8];     /* The salt value is 8 bytes. */
    unsigned char iv[16];      /* IV: 16 bytes */
    unsigned char reserve[8];  /* Reserved 8 bytes */
    WsecUint32    ciphertextLen;
} SdpCipherHeaderEx;

typedef struct {
    WsecUint32    hmacAlgId;  /* HMAC algorithm */
    unsigned char salt[8];    /* The salt value of the generated key is 8 bytes. */
    unsigned char reserve[4]; /* 4 bytes are reserved. */
    WsecUint32    hmacLen;    /* Indicates the length of an HMAC. */
} SdpHmacHeaderEx;
#pragma pack()

/* API Function Prototype Declaration */
/* Specify the domain and encryption algorithm ID, encrypt the specified plaintext, and obtain the ciphertext. */
unsigned long SdpEncryptEx(WsecUint32 domain, WsecUint32 algId,
    const unsigned char *plainText, WsecUint32 plaintextLen,
    unsigned char *ciphertext, WsecUint32 *ciphertextLen);

/* Decrypts the specified ciphertext data. The data with or without the HMAC can be decrypted. */
unsigned long SdpDecryptEx(WsecUint32 domain,
    const unsigned char *ciphertext, WsecUint32 ciphertextLen,
    unsigned char *plainText, WsecUint32 *plaintextLen);

/*
 * Specify the domain and encryption/MAC algorithm ID, encrypt the specified plaintext,
 * calculate the MAC, and obtain the ciphertext.
 */
unsigned long SdpEncryptWithHmacEx(WsecUint32 domain,
    WsecUint32 cipherAlgId, WsecUint32 hmacAlgId,
    const unsigned char *plainText, WsecUint32 plaintextLen,
    unsigned char *ciphertext, WsecUint32 *ciphertextLen);

/*
 * Specify the domain and encryption/MAC algorithm ID, encrypt the specified plaintext,
 * calculate the MAC, and obtain the ciphertext.
 * Note
 * This interface is used to resolve the problem that the HMAC key length of the SdpEncryptWithHmacEx
 * interface is insufficient, but the interface is incompatible.
 * The application scenarios of the SDP_DecryptEx interface in the old version are as follows,
 * 1. In the scenario where the KMC is deployed on multiple nodes (only V2 supports the deployment on multiple nodes),
 * this interface is used for newly deployed nodes.
 * In the scenario where SdpEncryptWithHmacV3 is used for encryption and the old node uses SDP_DecryptEx
 * in the old version for decryption,
 * SdpEncryptWithHmacV3 is incompatible with SDP_DecryptEx in earlier versions.
 * SDP_DecryptEx cannot decrypt data encrypted by SdpEncryptWithHmacV3.
 * SdpEncryptWithHmacV3
 * 2. In the version rollback scenario, after the data encrypted using SdpEncryptWithHmacV3 is rolled
 * back to the source version,
 * SDP_DecryptEx Failed to Decrypt the Newly Encrypted SdpEncryptWithHmacV3 Data
 * Therefore, for a single-node system, SdpEncryptWithHmacV3 is upgraded for multiple nodes,
 * or only old nodes are encrypted.
 * In the scenarios where new nodes are deployed for decryption, the SdpEncryptWithHmacV3 encryption
 * algorithm that cannot be decrypted after the rollback is considered.
 * The new SdpEncryptWithHmacV3 interface should be used after the risk of the output data is eliminated.
 */
unsigned long SdpEncryptWithHmacV3(WsecUint32 domain,
    WsecUint32 cipherAlgId, WsecUint32 hmacAlgId,
    const unsigned char *plainText, WsecUint32 plaintextLen,
    unsigned char *ciphertext, WsecUint32 *ciphertextLen);

/* Obtains the length of the ciphertext data corresponding to the plaintext data when HMAC is not calculated. */
unsigned long SdpGetCipherDataLenEx(WsecUint32 plaintextLen, WsecUint32 *ciphertextLenOut);

/* Obtains the length of the ciphertext data corresponding to the plaintext data when HMAC is calculated. */
unsigned long SdpGetCipherDataLenWithHmacEx(WsecUint32 plaintextLen, WsecUint32 *ciphertextLenOut);

/* Obtains the header structure of the input ciphertext. */
unsigned long SdpGetCipherHeaderEx(const unsigned char *ciphertext, WsecUint32 ciphertextLen,
    SdpCipherHeaderEx *cipherHeader);

/* Obtain MKInfo based on the obtained ciphertext information. */
unsigned long SdpGetMkDetailByCipher(const unsigned char *cipherData, WsecUint32 cipherDataLen, KmcMkInfo *mkInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* KMC_SRC_SDP_SDPV2_ITF_H */
