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
#include "common/Path.h"
#ifndef WIN32
#include <libgen.h>
#endif
#include "securec.h"

using namespace std;

namespace Module {
#ifdef WIN32
CPath CPath::m_instance;
CPath& CPath::GetInstance()
{
    return m_instance;
}
#else
CPath CPath::m_instance;
#endif

/* ------------------------------------------------------------
Function Name: Init
Description  : 初始化，进程入口处调用，用于获取agent程序路径
Others       :-------------------------------------------------------- */
int CPath::Init(const string& pszBinFilePath)
{
    char* pszTmp = NULL;
    char acAgentRoot[MAX_FULL_PATH_LEN] = {0};
    char tmpFilePath[MAX_FULL_PATH_LEN] = {0};
    uint32_t iLastIndex;
    pszBinFilePath.copy(tmpFilePath, MAX_FULL_PATH_LEN);
    tmpFilePath[MAX_FULL_PATH_LEN - 1] = '\0';
#ifdef WIN32
    int iRet = GetFullPathName(tmpFilePath, MAX_FULL_PATH_LEN, acAgentRoot, &pszTmp);
    if (iRet == 0) {
        return FAILED;
    }
    *pszTmp = 0;

    iLastIndex = strlen(acAgentRoot) - 1;
    acAgentRoot[iLastIndex] = 0;
    pszTmp = strrchr(acAgentRoot, '\\');
    iLastIndex = pszTmp - acAgentRoot;
    acAgentRoot[iLastIndex] = 0;
#else
    char* pszPath = dirname(tmpFilePath);
    if (pszPath == NULL) {
        return FAILED;
    }
    // CodeDex误报，Buffer Overflow，路径长度不会超过300
    if (strncpy_s(acAgentRoot, sizeof(acAgentRoot), pszPath, strlen(pszPath)) != EOK) {
        return FAILED;
    }

    pszTmp = strrchr(acAgentRoot, '/');
    // 如果直接执行rdagent执行程序，则dirname获取的路径为"."，这里strrchr返回NULL
    if (pszTmp == NULL) {
        return FAILED;
    }
    iLastIndex = pszTmp - acAgentRoot;
    acAgentRoot[iLastIndex] = 0;
#endif
    m_strAgentRootPath = acAgentRoot;
    return SUCCESS;
}

} // namespace Module