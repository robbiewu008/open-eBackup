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
#include "common/FileSearcher.h"
#include "common/Defines.h"
#include "common/File.h"
#include "common/MpString.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "securec.h"
using namespace std;
namespace {
const mp_int32 FILE_SEARCHER_NUM_1024 = 1024;
}

/* ------------------------------------------------------------
Description  :设置路径
Input        :     pszPath---路径
------------------------------------------------------------- */
mp_void CFileSearcher::SetPath(const mp_string& pszPath)
{
    m_paths.clear();
    const mp_char* ptr = pszPath.c_str();
    const mp_char* p = NULL;
    std::size_t sz = 0;
    while (*ptr) {
        while (*ptr == ' ') {
            ptr++;
        }

        p = ptr;
        sz = 0;
        while (*ptr && *ptr != PATH_SEPCH) {
            ptr++;
            sz++;
        }

        if (*ptr) {
            ptr++;
        }
        mp_string sp(p);
        AddPath(sp, sz);
    }
    RebuildPathString();
}
/* ------------------------------------------------------------
Description  :获取路径
Return       :   m_strPath ---获得的路径
------------------------------------------------------------- */
const mp_string& CFileSearcher::GetPath()
{
    return m_strPath;
}
/* ------------------------------------------------------------
Description  :清楚路径信息
------------------------------------------------------------- */
mp_void CFileSearcher::Clear()
{
    m_paths.clear();
    m_strPath = "";
}
/* ------------------------------------------------------------
Description  :根据路径查找文件是否存在
Input        :      pszFile---文件名，strPath---路径
Return       :    MP_TRUE---查找成功
                 MP_FALSE---查找失败
------------------------------------------------------------- */
mp_bool CFileSearcher::Search(const mp_string& pszFile, mp_string& strPath)
{
    COMMLOG(OS_LOG_DEBUG, "Begin search file, name %s.", BaseFileName(pszFile).c_str());

    for (std::size_t i = 0; i < m_paths.size(); i++) {
        if (IsExists(pszFile, m_paths[i], strPath)) {
            COMMLOG(OS_LOG_DEBUG, "Search file succ.");
            return MP_TRUE;
        }
    }

    COMMLOG(OS_LOG_ERROR, "Search file failed.");

    return MP_FALSE;
}
/* ------------------------------------------------------------
Description  :根据路径查找多个文件是否存在
Input        :      pszFile---文件名，flist---文件信息列表
Return       :  true---查找成功
                  false---查找失败
------------------------------------------------------------- */
mp_bool CFileSearcher::Search(const mp_string& pszFile, vector<mp_string>& flist)
{
    std::size_t sz = 0;
    mp_string strPath;

    for (std::size_t i = 0; i < m_paths.size(); i++) {
        if (IsExists(pszFile, m_paths[i], strPath)) {
            flist.push_back(strPath);
            sz++;
        }
    }

    return (sz > 0);
}
/* ------------------------------------------------------------
Description  :添加路径
Input        :      pszPath---路径
------------------------------------------------------------- */
mp_void CFileSearcher::AddPath(const mp_string& pszPath)
{
    AddPath(pszPath, pszPath.length());
    RebuildPathString();
}
/* ------------------------------------------------------------
Description  :添加 路径信息
Input        :      pszDir---路径，sz---路径字符长度
------------------------------------------------------------- */
mp_void CFileSearcher::AddPath(const mp_string& pszDir, std::size_t sz)
{
    mp_bool bFlag = pszDir.c_str()[0] == '\0' || sz == 0;
    if (bFlag) {
        return;
    }

    const mp_char* p = pszDir.c_str();
    while (sz > 0 && *p == ' ') {
        p++;
        sz--;
    }

    while (sz > 0 && p[sz - 1] == ' ') {
        sz--;
    }

    if (sz == 0) {
        return;
    }

    while (sz > 0 && IS_DIR_SEP_CHAR(p[sz - 1])) {
        if (sz == 1) {
            break;
        }

        sz--;
    }

    m_paths.push_back(mp_string(p, sz));
}
/* ------------------------------------------------------------
Description  :判断文件是否存在
Input        :      pszFile---文件，strDir---目录
Output       :   strPath---路径
------------------------------------------------------------- */
mp_bool CFileSearcher::IsExists(const mp_string& pszFile, const mp_string& strDir, mp_string& strPath)
{
    mp_char szFile[FILE_SEARCHER_NUM_1024] = {0};
    mp_int32 iRet;

    iRet = snprintf_s(szFile, sizeof(szFile), sizeof(szFile) - 1, "%s%s%s",
        strDir.c_str(), PATH_SEPARATOR.c_str(), pszFile.c_str());
    if (iRet == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "Snprintfs failed.");
        return MP_FALSE;
    }

    mp_string tmpStr(szFile);
    if (!CMpFile::FileExist(tmpStr)) {
        COMMLOG(OS_LOG_ERROR, "File not exist, file %s.", BaseFileName(tmpStr).c_str());
        return MP_FALSE;
    }

    strPath = szFile;
    return MP_TRUE;
}
/* ------------------------------------------------------------
Description  :重建路径为字符串格式
------------------------------------------------------------- */
mp_void CFileSearcher::RebuildPathString()
{
    m_strPath = CMpString::StrJoin(m_paths, STR_COLON);
}
