/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: KMC internal interface header file, which is not open to external systems.
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 * 2019-04-09 Zhang Jie (employee ID: 00316590): Added some interfaces to adapt to KMC 3.0.
 */

#ifndef KMC_SRC_KMC_KMCV2_PRI_H
#define KMC_SRC_KMC_KMCV2_PRI_H

#include "wsecv2_type.h"
#include "wsecv2_array.h"
#include "wsecv2_share.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define KMC_KCF_VER 1 /* KMC configuration file version */

#define KMC_RMK_LEN         32 /* RMK key length */
#define KMC_EK4MKF_LEN      32 /* MKF (MK file) encryption key length */
#define KMC_KEY4HMAC_LEN    32 /* Key length for HMAC */
#define KMC_HMAC_SHA256_LEN 32 /* Length of the SHA256 calculation result */
#define KMC_HASH_SHA256_LEN 32 /* HMACSHA256 calculation result length */
#define WSEC_HASH_LEN_MAX   64 /* Maximum HMAC result length */
#define WSEC_HMAC_LEN_MAX   64 /* Maximum HMAC result length */

/* CBB-dedicated domain */
#define KMC_PRI_DOMAIN_ID_MIN 1024
#define KMC_PRI_DOMAIN_ID_MAX 1056

#define KMC_ALL_DOMAIN (KMC_PRI_DOMAIN_ID_MAX + 1000)
#define KMC_PARTIAL_DOMAIN (KMC_PRI_DOMAIN_ID_MAX + 1001)
#define KMC_INVALID_DOMAIN (WsecUint32)(-1)
/* valid domainId value range */
#define KMC_DOMAINID_OUT_OF_RANGE(domainId) \
    (((domainId) != KMC_ALL_DOMAIN) && ((domainId) >= KMC_PRI_DOMAIN_ID_MIN))

#define KMC_KEYTYPE_MAX_LIFEDAYSEX (1024 * 1024) /* Maximum validity period of the key, preventing key reversal. */
#define KMC_FLAG_LENGTH 32 /* Length of the KMC file header identifier. The length is 32 bytes. */

#define KMC_MATERIAL_SIZE 32
#define MATERIAL_COUNT 2
#define KMC_DOMAIN_OVERFLOWKEYID  (WsecUint32)(-1)

#define KMC_KSF_CACHE_COUNT 4

#define KMC_MASKED_KEY_LEN 128

#define KMC_KSF_NUM 2
#define KMC_KSF_WITH_THIRD_NUM  (KMC_KSF_NUM + 1)  /* KSF NUM After Enable Third backup */

#define MASTER_KSF_INDEX 0
#define BACKUP_KSF_INDEX (MASTER_KSF_INDEX + 1)
#define THIRD_KSF_INDEX (MASTER_KSF_INDEX + 2)

#define KMC_ENCRYPT_MK_ALGID  WSEC_ALGID_AES256_CBC  /* MK Encryption Algorithm ID */
#define KMC_HMAC_MK_ALGID     WSEC_ALGID_HMAC_SHA256 /* HMAC protection MK data algorithm ID */

#define DEFAULT_KEY_LIFE_DAYS 180 /* Default MK validity period (days) */
#define DEFAULT_KEY_LEN 32 /* Default key length */
#define DEFAULT_ROOT_KEY_VALIDITY 3650 /* The default validity period of an RK is 10 years. */
#define DEFAULT_ROOT_KEY_RMK_ITERATION 10000 /* Number of iterations for root key derivation */

/* KMC lock protection type */
typedef enum {
    KMC_LOCK_NONE     = 0, /* No lock */
    KMC_LOCK_CFG      = 1, /* Lock configuration */
    KMC_LOCK_KEYSTORE = 2, /* Locking the Keystore */
    KMC_LOCK_BOTH     = 3  /* Lock configuration and Keystore */
} KmcLockType;

typedef enum {
    WSEC_WAIT_INIT = 0, /* Uninitialized */
    WSEC_INIT_FAIL,     /* Initialization failed. */
    WSEC_RUNNING,       /* Normal running */
    WSEC_ON_INIT        /* Initializing */
} WsecRunState; /* Status of the CBB */

typedef enum {
    KMC_MIN_KEYID = 1,
    KMC_MAX_KEYID
} KmcKeyIdEndType;

typedef enum {
    WSEC_FUNC_UNREG = 0,
    WSEC_FUNC_REG
} WsecFuncRegState;

typedef enum {
    KMC_COMPARE_EQ = 0,
    KMC_COMPARE_NOT_EQ,
    KMC_COMPARE_ALL
} KmcCompareResult;

typedef struct TagWsecFuncRegStatus {
    WsecUint32 state;
} WsecFuncRegStatus;

/* Root Key (RK) data structure */
#pragma pack(1)
    typedef struct TagKmcRkParameters {
    unsigned char rkMaterialA[KMC_MATERIAL_SIZE]; /* Root key material 1, which is fixed to 32 bytes. */
    unsigned char rkMaterialB[KMC_MATERIAL_SIZE]; /* Root key material 2. The value is fixed to 32 bytes. */
    unsigned char reserve[32]; /* 32 bytes are reserved. */
    unsigned char rmkSalt[32]; /* Derived RMK salt value, which is fixed at 32 bytes. */
} KmcRkParameters; /* Basic parameters for exporting the root key */
#pragma pack()

#pragma pack(1)
typedef struct TagKmcKsfRk {
    KmcRkAttributes rkAttributes;  /* RK basic attributes */
    KmcRkParameters rkParameters;  /* RootKey construction parameters */
    WsecUint32      mkNum;         /* Total number of master keys */
    WsecUint32      updateCounter;
    WsecUint32      mkRecordLen; /* Maximum length of each MK ciphertext. This parameter is extended in KSFV3. */
    /*
     * Number of MK updates in the shared domain,
     * which is used to determine the host identity (new master) during startup.
     */
    WsecUint32      sharedMkUpdateCounter;
    unsigned char   reserve[24];   /* Reserved 24 bytes */
    unsigned char   aboveHash[32]; /* SHA256 result, 32 bytes */
} KmcKsfRk; /* RootKey information in Keystore */
#pragma pack()

/* Master Key (MK) data structure */
#pragma pack(1)
typedef struct TagKmcMkRear {
    WsecUint32    plaintextLen; /* MK plaintext length */
    /*
     * Key (When the file is stored, the key is the ciphertext encrypted by the RMK.
     * When the file is stored in the memory, the key is the plaintext decrypted by the RMK.)
     */
    unsigned char key[WSEC_MK_LEN_MAX];
} KmcMkRear; /* MK trailer information */
#pragma pack()

#pragma pack(1)
typedef struct TagKmcKsfMk {
    KmcMkInfo     mkInfo;        /* Basic MK information */
    WsecUint32    cipherAlgId;   /* Encryption Method */
    unsigned char reserve[40];   /* 40 bytes are reserved. */
    /* The MK is encrypted using the RMK to store the IV.The length of the IV is 16 bytes. */
    unsigned char iv[16];
    WsecUint32    ciphertextLen; /* Length of the ciphertext of the MK */
    union {
        struct { /* V2 Member */
            KmcMkRear     mkRear;        /* MK tail information */
            unsigned char mkHash[32];    /* HMAC-SHA256, 32 bytes */
        } mkV2;
        struct { /* V3 Member */
            WsecUint32    plaintextLen;    /* MK plaintext length */
            /*
             * In V3, only the first 160 bytes are stored. The remaining bytes are followed by ciphertextLen.
             * In V3, if the hardware directly encrypts the MK,
             * the ciphertext must contain both the ciphertext and MAC address.
             * In V3, if the MK is encrypted using the software-layer root key,
             * AES-GCM is used. The ciphertext contains the tag.
             */
            unsigned char key[160]; /* In V3, 160 bytes of the original 128-byte key and 32-byte HMAC are used */
                                    /* as the first 160 bytes of the key ciphertext. */
        } mkV3;
    } v2orV3;
} KmcKsfMk; /* MK information stored in the KSF (not stored in the memory) */
#pragma pack()

#pragma pack(1)
typedef struct TagKmcKsfHmac {
    union {
        unsigned char      hashData[WSEC_HASH_LEN_MAX];
        unsigned long long partHash;
    } HashData;
    union {
        unsigned char      hashHmac[WSEC_HMAC_LEN_MAX];
        unsigned long long partHmac;
    } HashHmac;
} KmcKsfHmac;
#pragma pack()

#pragma pack(1)
typedef struct TagKmcMemMk {
    KmcMkInfo     mkInfo; /* Basic MK information */
    KmcMkRear     mkRear; /* MK tail information */
    /* Unique identifier of a key, which is the first eight bytes of the SHA256 value. */
    unsigned char hashData[WSEC_MK_HASH_REC_LEN];
} KmcMemMk; /* MK information stored in the memory or written to the MKF after encryption */
#pragma pack()

#pragma pack(1)
typedef struct TagKmcSyncMkHeader {
    WsecUint16 msgType;
    WsecUint16 version;
    WsecUint32 msgLen;
    WsecUint32 sharedMkUpdateCounter;
} KmcSyncMkHeader;
#pragma pack()

#pragma pack(1)
typedef struct TagKmcSyncMk {
    KmcMkInfo mkInfo; /* Basic MK information */
    KmcMkRear mkRear; /* MK tail information */
} KmcSyncMk;
#pragma pack()

typedef struct TagKmcKsfHardRk {
    WsecBool hasHardRk;
    WsecBuff hrkInfo;    /* Hardware root key access information */
    WsecBuff srkInfo;    /* Masked key encryption result by the hardware root key */
} KmcKsfHardRk;

typedef union TagKmcMaskedKey {
    /*
     * Plaintext key at the software layer, encrypted and decrypted by the hardware key,
     * and masked plaintext stored in the memory
     */
    unsigned char maskedKey[KMC_MASKED_KEY_LEN];
    struct {
        /* 32-byte software layer root key. This parameter is valid when HASSOFTLEVELRK is set to TRUE. */
        unsigned char softLevelRk[32];
        unsigned char ksfHmacKey[32];  /* KSF integrity verification key (32 bytes) */
        unsigned char reserve[64];     /* Reserved 64 bytes are used to fill in random numbers. */
    } tempName;
} KmcMaskedKey;

typedef struct TagKmcHardRkMem {
    WsecHandle   hwRkHandle; /* Hardware root key handle */
    KmcKsfHardRk hardRk;
    WsecUint32   refCount;
    KmcMaskedKey key;
} KmcHardRkMem;

typedef struct TagKmcKsfMem {
    char         *fromFile;      /* Keystore file from which the data comes */
    /*
     * MK array. Its elements store addresses of the KmcMemMk type.
     * Domain IDs and key types can be sorted in ascending order.
     * The domain ID and key ID must be unique.
     */
    WsecArray     mkArray;
    WsecUint32    updateCounter; /* Number of keystore file update rounds. */
    /*
     * Number of times that the master key in the shared domain is updated, which is used to determine
     * the host identity (the new one is the master one) during startup.
     */
    WsecUint32    sharedMkUpdateCounter;

    /* The following fields are extended in V3: */
    unsigned char ksfHash[WSEC_HASH_LEN_MAX];
    KmcKsfRk      rk;         /* KmcKsfRk */
    KmcKsfHardRk  hardRk;     /* Read HRKINFO and SRKINFO from KSF and load them to the hardware key handle. */
} KmcKsfMem; /* KSF file in the memory */

/* Type of the key to be obtained. */
typedef struct TagKmcKeyTypesInfo {
    int        typeCount; /* The value ranges from 1 to 3. */
    KmcKeyType keyTypes[KMC_KEY_TYPE_MAX - 1]; /* For details about the three key types, see KmcKeyType. */
} KmcKeyTypesInfo;

/* Active Key's domain and key type info */
typedef struct {
    WsecUint32 domainId;
    KmcKeyTypesInfo keyTypes;
} KmcActiveKeyParam;

/* KMC configuration data structure (memory) */
typedef struct TagKmcDomainCfg {
    KmcCfgDomainInfo domainInfo;
    WsecArray        keyTypeCfgArray; /* KeyType array, element KmcCfgKeyType type */
} KmcDomainCfg;

typedef struct TagKmcCfg {
    /* RK configuration information (valid only when the RK is generated in the CBB) */
    KmcCfgRootKey       rkCfg;
    KmcCfgKeyManagement keyManagementCfg;  /* Key management parameters */
    KmcCfgDataProtect   dataProtectCfg[3]; /* Data protection configuration, including 3 types */
    WsecArray           domainCfgArray;    /* Domain array. The type of the element is KmcDomainCfg. */
} KmcCfg;

typedef struct TagKmcSys {
    char        *keystoreFile[KMC_KSF_NUM];             /* Keystore file name, which is backed up in two copies. */
    WsecUint32  role;                        /* Identity information */
    WsecUint32  state;                       /* KMC Status */
    /* The following fields are extended in V3: */
    WsecBool    isHardware;                  /* Hardware-Protected Root Key */
    WsecBool    hasSoftLevelRk;              /* Include Software-Layer Root Key */
    WsecBool    enableThirdBackup;           /* enable the third backup path, default false; */
    WsecBool    deleteKsfOnInitFailed;       /* delete ksf when init failed */
    char        *keystoreBackupFile;
} KmcSys;

/*
 * MK file (MKF for short) data structure (for key export/import)
 * Format:
 * 0. 32-byte format code
 *     1. KmcMkfHeaderWithHmac
 * 2. N MK data ciphertexts (KmcMkFileOneMk)
 * 3. HMAC of all MK ciphertexts
 */
#pragma pack(1)
typedef struct TagKmcMkfHeader {
    WsecUint16    version;           /* MK file version */
    WsecUint16    ksfVersion;        /* Keystore file version */
    WsecUint32    cipherAlgId;       /* Encryption algorithm ID */
    WsecUint32    iterForEncKey;     /* Number of iterations during encryption key generation */
    unsigned char saltForEncKey[16]; /* Salt used for generating the encryption key. The value is fixed to 16 bytes. */
    unsigned char ivForEncMk[16];    /* IV used for encrypting the MK. The value is fixed to 16 bytes. */
    unsigned char reserveA[16];      /* 16 bytes are reserved. */

    WsecUint32    hmacAlgId;          /* HMAC algorithm ID */
    WsecUint32    iterForHmacKey;     /* Number of iterations during HMAC key generation */
    unsigned char saltForHmacKey[16]; /* Salt used for generating the HMAC key. The value is fixed to 16 bytes. */
    WsecUint32    cipherLenPerMk;     /* Length of a single MK ciphertext */
    WsecUint32    mkNum;              /* Number of MKs */
    unsigned char reserveB[16];       /* 16 bytes are reserved. */
} KmcMkfHeader; /* MK file header */
#pragma pack()

#pragma pack(1)
typedef struct TagKmcMkfHeaderWithHmac {
    KmcMkfHeader  mkfHeader;
    unsigned char hmacData[KMC_HMAC_SHA256_LEN]; /* HMAC of the preceding data */
} KmcMkfHeaderWithHmac;
#pragma pack()

#pragma pack(1)
typedef struct TagKmcMkFileOneMk {
    KmcMkInfo     mkInfo;                     /* Basic MK information */
    /* The MK uses the RMK to encrypt the IV and has a fixed length of 16 bytes. */
    unsigned char iv[16];
    WsecUint32    plaintextLen;               /* Plaintext key length */
    unsigned char plainText[WSEC_MK_LEN_MAX]; /* Plaintext key */
} KmcMkFileOneMk; /* MK records in the MK file (encrypted storage) */
#pragma pack()

/*
 * Obtain the MK based on the domain ID and key ID. If the MK cannot be obtained,
 * obtain it based on the hash. Ensure that the hash values are consistent.
 */
unsigned long KmcGetMkByIDHash(WsecUint32 domainId,
    WsecUint32 keyId,
    const unsigned char *hashData, WsecUint32 hashLen,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen);

/* Obtains the currently effective key, key ID, and hash value in a specified domain. */
unsigned long KmcGetActiveMkWithHash(WsecUint32 domainId,
    unsigned char *keyBuff, WsecUint32 *keyBuffLen,
    WsecUint32 *keyId, unsigned char *keyHash, size_t hashLen);

/*
 * This interface is used to obtain a valid key in a specified domain.
 * If multiple keys in a domain are valid, any of them can be obtained.
 * This function also returns the status information of the key.
 * The creation time and expiration time in the information is obtained based on the local time function,
 * The invoker can determine whether the certificate is trusted based on the site requirements.
 * The default expiration time is 180 days. If there are multiple certificates in the keystore,
 * This function is used to obtain a random key from the keys with the same domain but different IDs but in valid state.
 */
unsigned long KmcPriGetActiveMk(WsecUint32 domainId, KmcKeyTypesInfo keyTypes, KmcMkInfo *mkInfo,
    unsigned char *keyPlaintextBuff, WsecUint32 *keyBuffLen);

/* Check whether the KSF version number is valid. */
WsecBool IsValidKsfVersion(WsecUint16 ksfVersion);

/* KSF V3 or Not */
WsecBool IsKsfV3(WsecUint16 ksfVersion);

/* KSF V1 or Not */
WsecBool IsKsfV1(WsecUint16 ksfVersion);

/* KSF V2 or Not */
WsecBool IsKsfV2(WsecUint16 ksfVersion);

/* Whether KSF V1 is used can be determined only based on KSF V1 and KSF V2. */
WsecBool IsKsfV1AndNotV2(WsecUint16 ksfVersion);

/* KSF V1 or V2 */
WsecBool IsKsfV1OrV2(WsecUint16 ksfVersion);

/* KSF V2 or V3 */
WsecBool IsKsfV2OrV3(WsecUint16 ksfVersion);

/* whether support sync mk by interface */
WsecBool IsSupportSyncMk(WsecUint16 ksfVersion);

/*
 * Calculated in network sequence (RK hash and MK HMAC V1 are calculated in host sequence,
 * and V2/V3 are calculated in network sequence.)
 */
WsecBool CalcByNetWorkOrder(WsecUint16 ksfVersion);

/* Initializes the PRI module.The PRI module manages the KmcSys structure and provides global information for the KMC */
unsigned long PriKmcSysInit(WsecUint32 state, const WsecInternalInitParam *initParam);

/* Obtain the KSF name based on the KSF index. */
char *PriKmcSysGetKsf(WsecUint32 idx);

/* Get the third ksf backup file name */
char *PriKmcSysGetKsfBackupKsf(void);

/* Enable the third ksf backup file */
WsecVoid PriKmcSysSetEnableThirdBackup(WsecBool isEnable);

/* Whether enabled the third ksf backup file */
WsecBool PriKmcSysGetIsEnableThirdBackup(void);

/* delete ksf backup file when init failed */
WsecBool PriKmcSysGetIsDeleteKsfOnInitFailed(void);

/* set delete ksf on startup feature */
WsecVoid PriKmcSysSetDeleteKsfOnInitFailed(WsecBool isEnable);

/* Obtains whether hardware protection is enabled (corresponding to the V3 KSF). */
WsecBool PriKmcSysGetIsHardware(void);

/* Sets the current KMC status. */
WsecVoid PriKmcSysSetState(WsecUint32 state);

/* Obtaining the Current KMC Status */
WsecUint32 PriKmcSysGetState(void);

/* Set the current KMC role. */
WsecVoid PriKmcSysSetRole(WsecUint32 role);

/* Obtaining the Current KMC Role */
WsecUint32 PriKmcSysGetRole(void);

/* Whether the root key of the software layer exists, which is determined by the initialization parameter */
WsecBool PriKmcSysGetHasSoftLevelRk(void);

/* Deinitializes the PRI module. */
WsecVoid PriKmcSysUninit(void);

/* init keystore files */
WsecUint32 PriKmcSysInitKsfName(const KmcKsfName *ksfName);

/* The byte order of the MK information is converted. */
WsecVoid CvtByteOrderForMkInfo(KmcMkInfo *mkInfo, WsecUint32 direction);

/* MK Synchronization Byte Sequence Conversion */
WsecVoid CvtByteOrderForSyncMk(KmcSyncMk *mk, WsecUint32 direction);

/* MK Synchronization Header Byte Sequence Conversion */
WsecVoid WsecCvtByteOrderForSyncMkHeader(KmcSyncMkHeader *header, WsecUint32 direction);

/* Converts the byte order of the header information in the MKF. */
WsecVoid CvtByteOrderForMkfHdr(KmcMkfHeader *mkfHeader, WsecUint32 direction);

/* The MK information word in the MKF is converted into the stanza. */
WsecVoid CvtByteOrderForMkfMk(KmcMkFileOneMk *mkfMk, WsecUint32 direction);

/* compare the mk domainid with the input domainid */
KmcCompareResult KmcCompareDomain(WsecUint32 compareDomain, WsecUint32 toCompareDomain);

/* Set the key time. */
unsigned long SetLifeTime(WsecUint32 lifeDays, WsecSysTime *createUtc, WsecSysTime *expireUtc);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* KMC_SRC_KMC_KMCV2_PRI_H */
