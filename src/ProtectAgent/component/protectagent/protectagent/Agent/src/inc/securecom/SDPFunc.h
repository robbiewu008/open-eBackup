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
#ifndef _SDP_FUNC_H_
#define _SDP_FUNC_H_

#include "common/Defines.h"

static const mp_string KMC_STORE_FILE = "kmc_store.txt";
static const mp_string KMC_STORE_FILE_BAK = "kmc_store_bak.txt";
static const mp_string KMC_CONFIG_FILE = "kmc_config.txt";
static const mp_string KMC_CONFIG_FILE_BAK = "kmc_config_bak.txt";

static const mp_uchar PRIVATE_MK_DOMAIN_ID = 2;  // the KMC domain for create mk from external input

static const mp_uchar PLAIN_TEXT_MIN_LEN = 32;
static const mp_uchar PLAIN_TEXT_MAX_LEN = 112;

static const mp_uchar MIN_MK_LIFE_DAYS = 30;
static const mp_int32 MAX_MK_LIFE_DAYS = 3650;

#define KMC_DOMAIN_DESC "The domain is used to create MK by importing plaintext from users input"

AGENT_API mp_int32 InitKmcByFile(const mp_string& kmcStoreFile, const mp_string& kmcStoreBakFile,
    const mp_uint32 roleType, const mp_string& kmcConfBakFile);
EXTER_ATTACK AGENT_API mp_int32 InitializeKmc(const mp_uint32 roleType);
AGENT_API mp_int32 FinalizeKmc();
AGENT_API mp_int32 ResetKmc();
AGENT_API mp_void TimerKmc();

AGENT_API mp_int32 EncryptStrKmc(const mp_string& inStr, mp_string& outStr);
AGENT_API mp_int32 DecryptStrKmc(const mp_string& inStr, mp_string& outStr);
AGENT_API mp_int32 GenFileHmacKmc(const mp_string& filePath, mp_string& fileHMAC);
AGENT_API mp_int32 VerifyFileHmacKmc(const mp_string& filePath, const mp_string& fileHMAC);

AGENT_API mp_int32 EncryptStrKmcWithReset(const mp_string& inStr, mp_string& outStr);
AGENT_API mp_int32 DecryptStrKmcWithReset(const mp_string& inStr, mp_string& outStr);
AGENT_API mp_int32 GenFileHmacKmcWithReset(const mp_string& filePath, mp_string& fileHMAC);
AGENT_API mp_int32 VerifyFileHmacKmcWithReset(const mp_string& filePath, const mp_string& fileHMAC);

AGENT_API mp_int32 CheckIfDomainExists(const mp_uint32& domainId);
AGENT_API mp_int32 RegisterExternalMk(const mp_string& plainTextKey, const mp_uint32& keyLifeDays);
AGENT_API mp_int32 GetExternalDomainInfo();
AGENT_API mp_int32 RegisterPrivateDomain();
AGENT_API mp_int32 CreateInternalMK();
AGENT_API mp_int32 PreCheck(const mp_string& inStr, mp_uint32& iLen);

#endif
