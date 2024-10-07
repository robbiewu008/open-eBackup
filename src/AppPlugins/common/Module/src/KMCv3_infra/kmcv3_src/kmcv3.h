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
#ifndef RC_SRC_KMCV3_INFRA_KMCV3_SRC_KMCV3_H
#define RC_SRC_KMCV3_INFRA_KMCV3_SRC_KMCV3_H

#include <string>
#include <functional>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif
enum KMC_DOMAIN {
    KMC_SHARE_INNER_DOMAIN = 0,
    KMC_SHARE_IMPORT_DOMAIN = 1,
    KMC_LOCAL_DOMAIN = 2,
};
enum KMC_RET {
    KMC_SUCESS = 0,
    KMC_FAIL   = 1,
    KMC_ENCTXT_INVAILD = 2,   /* 密文无效 */
};

struct MkStatus {
    unsigned int domainID;
    unsigned int keyID;
    unsigned int keyStatus;
    unsigned int isExpire;
};

/*
 * Function: InitKMCV3c
 * Input:  const char * storeFilePath,         存储路径
           const char * storeBakFilePath,      备份文件路径
           const char * moduleName             模块名，建议字母，数字，下划线
 * Output: nullptr
 */
KMC_RET InitKMCV3c(const char *storeFilePath, const char *storeBakFilePath, const char *moduleName);

/*
 * Function: EncryptV3c
 * Description: 加密函数，v3版本，C接口
 * Input:  KMC_DOMAIN domain,
 * Input:  KMC_DOMAIN domain,
           const char *plainTxt,  明文
 * Output: char **encText         密文
 * Return: ture/false
 * Others: 输出的密文内存，需要调用Kmc_Free释放
 */
KMC_RET EncryptV3c(KMC_DOMAIN domain, const char *plainTxt, char **encText);

/*
 * Function: DecryptV3c
 * Description: 解密函数，v3版本，C接口
 * Input:  KMC_DOMAIN domain,
 * Input:  KMC_DOMAIN domain,
           const char *encTextB64, 密文
 * Output: char **plainTxt         明文
 * Return: ture/false
 * Others: 输出的明文内存，需要调用Kmc_Free释放
 */
KMC_RET DecryptV3c(KMC_DOMAIN domain, char **plainTxt, const char *encTextB64);

/*
 * Function: Decrypt
 * Description: 解密函数，v1版本，C接口
 * Input:  KMC_DOMAIN domain,
 * Input:  KMC_DOMAIN domain,
           const char *encTextB64, 密文
 * Output: char **plainTxt         明文
 * Return: ture/false
 * Others: 输出的明文内存，需要调用Kmc_Free释放
 */
KMC_RET DecryptPwdV1(KMC_DOMAIN domain, std::string& outStr, const std::string& inStr);
/*
 * Function: DeInitKmc
 * Input:  nullptr
 * Output: nullptr
 */
unsigned long  DeInitKmc(void);

/*
 * Function: Kmc_Free
 * Input:  char *mem  内存指针
 * Output: nullptr
 */
void KmcFree(char *mem);

#ifdef __cplusplus
    }
#endif

#endif