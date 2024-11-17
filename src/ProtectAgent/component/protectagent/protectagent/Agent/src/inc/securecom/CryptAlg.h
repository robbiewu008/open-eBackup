/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#ifndef _CRYPT_ALG_H_
#define _CRYPT_ALG_H_

#include "common/Defines.h"
#ifdef WIN32
#include <Wincrypt.h>
#endif

// 密钥分开存放，增加破解难度
static const int AES_KEY_LEN = 16;
static const int AES_IV_LEN = 32;

static const mp_uchar PBKDF_KEY_LEN = 64;
static const mp_int32 PBKDF_ITER_TIMES = 10000000;
static const mp_uchar CIPHER_BUFF_LEN = 128;
static const mp_uchar PBKDF_SALT_MAX_LEN = 40;

static const mp_int32 NOT_KMC_CIPHER_TEXT = 353;
static const mp_uchar INPUT_BUFF_LEN_NOT_ENOUGH = 153;
static const mp_uint32 KMC_ROLE_TYPE_AGNET = 0;
static const mp_uint32 KMC_ROLE_TYPE_MASTER = 1;

typedef enum {
    CRYPT_ENCYP_AES = 0,  // AES加密
    CRYPT_DECYP_AES,      // AES解密
    CRYPT_SHA,            // sha 256
    CRYPT_SALT,           // 计算盐值
    CRYPT_SECOND,         // 计算秒数
    CRYPT_PBKDF2          // PBKDF2算法
} CRYPT_ALG;

const std::size_t SHA256_BLOCK_SIZE = 64;

// SHA256
mp_int32 AGENT_API GetSha256Hash(const mp_string& buff, const std::size_t len, mp_string& hashHex, std::size_t hexLen);

// PBKDF2
mp_int32 AGENT_API PBKDF2Hash(const mp_string& strPlainText, const mp_string& strSalt, mp_string& strCipherText);

// RANDOM Generate 8 bytes random num, default not use preudo random number, use true random number.
mp_int32 AGENT_API GetRandom(mp_uint64& num, bool notPreudo = true);

// RANDOM SALT STRINMG
mp_int32 AGENT_API GenRandomSalt(mp_string& saltstring);

// Init crypt
mp_int32 AGENT_API InitCrypt(const mp_uint32 roleType);
mp_int32 AGENT_API InitCryptByFile(const mp_string& kmcStoreFile, const mp_string& kmcStoreBakFile,
    const mp_uint32 roleType, const mp_string& kmcConfBakFile);

// Finalize crypt
mp_int32 AGENT_API FinalizeCrypt();

// Call crypt timer
mp_void AGENT_API CallCryptTimer();

// Encrypt string
mp_void AGENT_API EncryptStr(const mp_string& inStr, mp_string& outStr);

// Decrypt string
mp_void AGENT_API DecryptStr(const mp_string& inStr, mp_string& outStr);
mp_void AGENT_API DecryptStrKMC(const mp_string& inStr, mp_string& outStr);

// Compute HMAC
mp_int32 AGENT_API ComputeHMAC(const mp_string& filePath, mp_string& fileHMAC);

// Verify HMAC
mp_int32 AGENT_API VerifyHMAC(const mp_string& filePath, const mp_string& fileHMAC);

// Register external MK
mp_int32 AGENT_API RegisterExternalMK(const mp_string& plainText, const mp_uint32 keyLifeDays);

mp_int32 AGENT_API RestartAgent();
// Manually Updating the KMC Key
mp_int32 AGENT_API ManualUpdateDmcKey();

#endif
