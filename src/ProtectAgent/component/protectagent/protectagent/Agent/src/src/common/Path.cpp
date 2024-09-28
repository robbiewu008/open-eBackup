#include "common/Path.h"
#ifndef WIN32
#include <libgen.h>
#endif
#include "securec.h"
#include "common/ErrorCode.h"
 
CPath CPath::m_instance;
CPath& CPath::GetInstance()
{
    return m_instance;
}
/* ------------------------------------------------------------
Function Name: Init
Description  : 初始化，进程入口处调用，用于获取agent程序路径
Others       :-------------------------------------------------------- */
mp_int32 CPath::Init(const mp_string& pszBinFilePath)
{
    mp_char tmpFilePath[MAX_FULL_PATH_LEN] = {0};
    pszBinFilePath.copy(tmpFilePath, MAX_FULL_PATH_LEN);
    tmpFilePath[MAX_FULL_PATH_LEN - 1] = '\0';
    mp_uint32 iLastIndex;
    mp_char* pszTmp = NULL;
    mp_char acAgentRoot[MAX_FULL_PATH_LEN] = {0};
#ifdef WIN32
    mp_int32 iRet = GetFullPathName(tmpFilePath, MAX_FULL_PATH_LEN, acAgentRoot, &pszTmp);
    if (iRet == 0) {
        return ERROR_COMMON_OPER_FAILED;
    }
    *pszTmp = 0;

    iLastIndex = strlen(acAgentRoot) - 1;
    acAgentRoot[iLastIndex] = 0;
    pszTmp = strrchr(acAgentRoot, '\\');
    iLastIndex = pszTmp - acAgentRoot;
    acAgentRoot[iLastIndex] = 0;
#else
    mp_char* pszPath = dirname(tmpFilePath);
    if (pszPath == NULL) {
        return ERROR_COMMON_OPER_FAILED;
    }
    // CodeDex误报，Buffer Overflow，路径长度不会超过300
    if (strncpy_s(acAgentRoot, sizeof(acAgentRoot), pszPath, strlen(pszPath)) != EOK) {
        return ERROR_COMMON_OPER_FAILED;
    }

    pszTmp = strrchr(acAgentRoot, '/');
    // 如果直接执行rdagent执行程序，则dirname获取的路径为"."，这里strrchr返回NULL
    if (pszTmp == NULL) {
        return ERROR_COMMON_OPER_FAILED;
    }
    iLastIndex = pszTmp - acAgentRoot;
    acAgentRoot[iLastIndex] = 0;
#endif
    m_strAgentRootPath = acAgentRoot;
    return MP_SUCCESS;
}
