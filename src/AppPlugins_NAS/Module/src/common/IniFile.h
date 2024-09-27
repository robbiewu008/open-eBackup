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
#ifndef MODULE_IN_FILE_H
#define MODULE_IN_FILE_H

#include <map>
#include "define/Types.h"
#include "common/Thread.h"

#ifdef WIN32
#include <shlwapi.h>
#endif

namespace Module {

typedef std::map<std::string, std::string> DKEYPAIR;
class CIniKeySection
{
private:
    DKEYPAIR m_Keys;
    std::string m_strName;

public:
    CIniKeySection(const char* pszName);
    ~CIniKeySection();

    void Print();
    const char* GetName();
    int Add(const char* pszText);
    const char* GetValue(const char* pszKeyName);
    int SetValue(const char* pszKeyName, const char* pszValue);
    DKEYPAIR GetKeys();

private:
    //bool IsValidKeyValue(const const char* pszText);
};

#ifdef WIN32
class AGENT_API CIniFile
#else
class CIniFile
#endif
{
    typedef std::map<std::string, CIniKeySection*> DSECTS;

private:
    thread_lock_t m_tLock;   // m_Sections访问互斥锁
    std::string m_strFileName; // 配置文件路径
    time_t m_LastMTime;
    DSECTS m_Sections;
    static CIniFile m_instance;   // 单例对象

public:
#ifdef WIN32
    static CIniFile& GetInstance();
#else
    static CIniFile& GetInstance()
    {
        return m_instance;
    }
#endif
    CIniFile();
    ~CIniFile();

    int Init(const std::string& strFileNamePath);
    int GetIntValue(const char* pszSectionName, const char* pszKeyName, int& iValue);
    int GetStrValue(const char* pszSectionName, const char* pszKeyName, std::string& strValue);
    int WriteStrValue(const char* pszSectionName, const char* pszKeyName, const char* pszValue);
    void Print();
    CIniKeySection* GetSection(const char* pszSection);

private:
    void Clear();
    int str2num(const std::string& strValue);
    bool IsModified();
    char* ReadLine(FILE* pFile, char* pszBuf, int iBufLen);
    bool IsValidText(const char* pszText);
    int CopySection(const char * str_src, char*  str_dest);
    int LoadSection(FILE* pFile, CIniKeySection* pSection, bool& bHasNextSection, char* pszNextSection,
        int iBufLen);
    //int Load(string& strConfFile, map<string, STRING_MAP>& rConfMap);
    int Load();
    int WriteToFile();
    bool ReadFileContxt(FILE* pFile);

};

} // namespace Module

#endif // IN_FILE_H