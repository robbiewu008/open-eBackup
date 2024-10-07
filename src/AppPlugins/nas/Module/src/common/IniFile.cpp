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
#include "IniFile.h"
#include <iostream>
#include <stdio.h>
#include "securec.h"
#include "common/Utils.h"
#include "common/File.h"
#include "common/MpString.h"
#include "log/Log.h"

using namespace std;

namespace Module {

const string MODULE_NAME = "IniFile";
#ifdef WIN32
CIniFile CIniFile::m_instance;
CIniFile& CIniFile::GetInstance()
{
    return m_instance;
}
#endif
CIniKeySection::CIniKeySection(const char* pszName)
{
    if (NULL != pszName)
    {
        m_strName = pszName;
    }
}

CIniKeySection::~CIniKeySection()
{

}

const char* CIniKeySection::GetName()
{
    return m_strName.c_str();
}

void CIniKeySection::Print()
{
    DKEYPAIR::iterator iterKey;

    for (iterKey = m_Keys.begin(); iterKey != m_Keys.end(); iterKey++)
    {
        printf("%s = %s\n", iterKey->first.c_str(), iterKey->second.c_str());
    }
}

const char* CIniKeySection::GetValue(const char* pszKeyName)
{
    char szKey[1024] = {0};
    int iRet = SUCCESS;

    iRet = strncpy_s(szKey, sizeof(szKey), pszKeyName, strlen(pszKeyName));
    if (SUCCESS != iRet) {
        return NULL;
    }

    CMpString::ToUpper(szKey);
    DKEYPAIR::iterator it = m_Keys.find(szKey);
    if (it != m_Keys.end()) {
        return it->second.c_str();
    }

    return NULL;
}

int CIniKeySection::SetValue(const char* pszKeyName, const char* pszValue)
{
    const int KEY_MAX_LENGTH = 1024; 
    char szKey[KEY_MAX_LENGTH + 1] = {0};
    int iRet = SUCCESS;
    string strValue = pszValue;
    if (strlen(pszKeyName) > KEY_MAX_LENGTH) {
        return FAILED;
    }

    iRet = strncpy_s(szKey, sizeof(szKey), pszKeyName, strlen(pszKeyName));
    if (SUCCESS != iRet) {
        return iRet;
    }

    CMpString::ToUpper(szKey);
    DKEYPAIR::iterator it = m_Keys.find(szKey);
    if (it != m_Keys.end()) {
        it->second = strValue;
        return SUCCESS;
    }

    return FAILED;
}

int CIniKeySection::Add(const char* pszText)
{
    string line(pszText);

    char szKey[MAX_LINE_SIZE] = {0};
    char szValue[MAX_LINE_SIZE] = {0};

    string::size_type pos = line.find("=");
    if (pos == string::npos) {
        HCP_Logger_noid(ERR, MODULE_NAME)<< "string format is error. string="<< line <<HCPENDLOG;
        return FAILED;
    }

    errno_t iRet = strncpy_s(szKey, sizeof(szKey), line.c_str(), pos);
    if (0 != iRet) {
        HCP_Logger_noid(ERR, MODULE_NAME)<< "string format is error. string="<< line <<HCPENDLOG;
        return iRet;
    }
    if (line.size() > pos + 1) {
        iRet = strncpy_s(szValue, sizeof(szValue), line.c_str() + pos +1, line.size() - pos -1);
        if (0 != iRet)
        {
            HCP_Logger_noid(ERR, MODULE_NAME)<< "string format is error. string="<< line <<HCPENDLOG;
            return iRet;
        }
    }

    CMpString::ToUpper(szKey);
    string tempSzKey = szKey;
    if (tempSzKey != "IAMPASSWD") {
        HCP_Logger_noid(DEBUG, MODULE_NAME)<<"add value." << GetName()
                << "." << szKey <<HCPENDLOG;
    } else {
        HCP_Logger_noid(DEBUG, MODULE_NAME)<<"add value." << GetName()
                << "." << WIPE_SENSITIVE(szKey) << "=****" <<HCPENDLOG;
    }
    char *szKeyPtr = szKey;
    char *szKeyTrim = CMpString::Trim(szKeyPtr);
    if (NULL == szKeyTrim || 0== strlen(szKeyTrim)) {
        HCP_Logger_noid(ERR, MODULE_NAME)<< "Add szKeyTrim is null or length is zero. "<<HCPENDLOG;
        return FAILED;
    }
    m_Keys[szKeyTrim] = szValue;

    return SUCCESS;
}

DKEYPAIR CIniKeySection::GetKeys()
{
    return m_Keys;
}

CIniFile::CIniFile()
{
    CMpThread::InitLock(&m_tLock);
    m_LastMTime = 0;
}

CIniFile::~CIniFile()
{
    Clear();
    CMpThread::DestroyLock(&m_tLock);
}

int CIniFile::Init(const string& strFileNamePath)
{
    int iRet = SUCCESS;
    m_LastMTime= 0;
    m_strFileName = strFileNamePath;
    iRet = Load();
    if (iRet != SUCCESS) {
        HCP_Logger_noid(ERR, MODULE_NAME) << "Init file failed! path="
                << strFileNamePath << HCPENDLOG;
        return FAILED;
    }

    return SUCCESS;
}

void CIniFile::Print()
{
    DSECTS::iterator iterSec;

    for (iterSec = m_Sections.begin(); iterSec != m_Sections.end(); iterSec++) {
        printf("[%s]\n", iterSec->first.c_str());
        ((CIniKeySection*)iterSec->second)->Print();
    }
}

int CIniFile::str2num(const string& strValue)
{
    if ("" == strValue || ('0' == strValue[0] && 1 < strValue.length())) {
        return FAILED;
    }
    for (int i = 0; i < strValue.length(); ++i) {
        if('0' > strValue[i] || '9' < strValue[i]) {
            return FAILED;
        }
    }
    int intValue = atoi(strValue.c_str());
    return intValue;
}

int CIniFile::GetIntValue(const char* pszSectionName, const char* pszKeyName, int& iValue)
{
    int iRet = SUCCESS;
    string strValue;

    if (NULL == pszSectionName || NULL == pszKeyName)
    {
        HCP_Logger_noid(ERR, MODULE_NAME) << "Params is NULL. "<< HCPENDLOG;
        return FAILED;
    }

    iRet = GetStrValue(pszSectionName, pszKeyName, strValue);
    if (SUCCESS != iRet)
    {
        return iRet;
    }

    iValue = str2num(strValue);
    if(FAILED == iValue)
    {
        iRet = FAILED;
    }

    return iRet;
}

int CIniFile::GetStrValue(const char* pszSectionName, const char* pszKeyName, string& strValue)
{
    int iRet = SUCCESS;
    CIniKeySection* pSection = NULL;
    DSECTS::iterator iterSec;

    if (NULL == pszSectionName || NULL == pszKeyName)
    {
        HCP_Logger_noid(ERR, MODULE_NAME) << "Params is NULL. "<< HCPENDLOG;
        return FAILED;
    }

    CThreadAutoLock tlock(&m_tLock);
    iRet = Load();
    if (SUCCESS != iRet)
    {
        HCP_Logger_noid(ERR, MODULE_NAME) <<"Load conf fil failed. path ="<< m_strFileName<< HCPENDLOG;
        return iRet;
    }

    pSection = GetSection(pszSectionName);
    if (NULL == pSection)
    {
        return FAILED;
    }

    //pSection->Print();
    const char* pszValue = pSection->GetValue(pszKeyName);
    if (NULL == pszValue)
    {
        return FAILED;
    }

    strValue = pszValue;
    return SUCCESS;
}

int CIniFile::WriteStrValue(const char* pszSectionName, const char* pszKeyName, const char* pszValue)
{
    int iRet = SUCCESS;
    CIniKeySection* pSection = NULL;

    if (NULL == pszSectionName || NULL == pszKeyName || NULL == pszValue) {
        HCP_Logger_noid(ERR, MODULE_NAME) << "param is NULL." <<HCPENDLOG;
        return FAILED;
    }

    CThreadAutoLock tlock(&m_tLock);
    iRet = Load();
    if (SUCCESS != iRet) {
        HCP_Logger_noid(ERR, MODULE_NAME)<<"Load conf file failed. path="
                <<m_strFileName<<HCPENDLOG;
        return iRet;
    }

    pSection = GetSection(pszSectionName);
    if (NULL == pSection) {
        HCP_Logger_noid(DEBUG, MODULE_NAME) <<"Can not get section. section="<< pszSectionName<<HCPENDLOG;
        return FAILED;
    }

    iRet = pSection->SetValue(pszKeyName, pszValue);
    if (SUCCESS != iRet) {
        HCP_Logger_noid(ERR, MODULE_NAME) <<"Set value failed, key"<<pszKeyName
                <<" value" << pszValue<<HCPENDLOG;
        return iRet;
    }

    return SUCCESS;
}

void CIniFile::Clear()
{
    DSECTS::iterator it = m_Sections.begin();
    for(; it != m_Sections.end(); it++)
    {
        delete it->second;
        it->second = NULL;
    }

    m_Sections.clear();
}

bool CIniFile::IsModified()
{
    int iRet = SUCCESS;

    if(m_strFileName.empty())
    {
        return false;
    }

    time_t tLastMoidfyTime;
    iRet = CFile::GetlLastModifyTime(m_strFileName.c_str(), tLastMoidfyTime);
    if (iRet != SUCCESS) {
        return false;
    }

    return (tLastMoidfyTime != m_LastMTime);
}

char* CIniFile::ReadLine(FILE* pFile, char* pszBuf, int iBufLen)
{
    if (NULL == pFile) {
        return NULL;
    }

    int ch;
    while((ch=fgetc(pFile)) == int('\r') || ch == int('\n')) {
        //do nothing
    }
    char* pRet = fgets(pszBuf + 1, iBufLen - 1, pFile);
    if (NULL == pRet) {
        return NULL;
    }

    pszBuf[0] = (char)ch;
    pszBuf[iBufLen - 1] = 0;
    while(*pRet != '\0') {
        if(*pRet == '\r' || *pRet == '\n') {
            *pRet = '\0';
        }
        pRet ++;
    }

    return pszBuf;
}

// 判断行数据是否合法
// 下面的行数据不是合法的：1. 空行，完全的空行或只有空格的行; 2. 注释行，以“#”或“;”开头的数据
bool CIniFile::IsValidText(const char* pszText)
{
    if (0 == pszText) {
        return false;
    }

    while (IS_SPACE(*pszText)) {
        pszText++;
    }

    if ('\0' == *pszText) {
        return false;
    }

    if ('#' == *pszText || ';' == *pszText) {
        return false;
    }

    return true;
}

CIniKeySection* CIniFile::GetSection(const char* pszSection)
{
    int iRet = SUCCESS;
    char szSName[1024] = {0};

    iRet = strncpy_s(szSName, sizeof(szSName), pszSection, strlen(pszSection));
    if (SUCCESS != iRet) {
        return NULL;
    }

    string sectionName = CMpString::ToUpper(szSName);
    DSECTS::iterator it = m_Sections.find(sectionName);
    if (it != m_Sections.end()) {
        return it->second;
    }

    return NULL;
}

int CIniFile::LoadSection(FILE* pFile, CIniKeySection* pSection, bool& bHasNextSection,
    char* pszNextSection, int iBufLen)
{
    if (NULL == pSection) {
        HCP_Logger_noid(ERR, MODULE_NAME) << "pSection is NULL" << HCPENDLOG;
        return FAILED;
    }

    if (NULL != GetSection(pSection->GetName())) {
        // 重复的SECTION
        return FAILED;
    }

    char* pstr = NULL;
    char szLineText[MAX_LINE_SIZE] = {0};
    bHasNextSection = false;
    while ((pstr = ReadLine(pFile, szLineText, sizeof(szLineText))) != 0) {
        if (!IsValidText(pstr)) {
            continue;
        }

        while(IS_SPACE(*pstr)) pstr++;
        if (*pstr == '[') {
            bHasNextSection = true;
            if (SUCCESS != strncpy_s(pszNextSection, iBufLen, pstr, strlen(pstr))) {
                return FAILED;
            }

            break;
        }

        if (SUCCESS != pSection->Add(pstr)) {
            HCP_Logger_noid(ERR, MODULE_NAME)<< "add failed. text = "<< pstr <<HCPENDLOG;
            return FAILED;
        }
    }

    m_Sections[pSection->GetName()] = pSection;

    return SUCCESS;
}

int CIniFile::CopySection(const char * pstr, char*  str_dest)
{
    int n = 0;

    // 搜索SECTION
    if (*pstr != '[') {
        HCP_Logger_noid(ERR, MODULE_NAME)<<"Search '[' faild, line.lineText="
            << pstr <<HCPENDLOG;
        // 搜索'['字符时遇到遇到非法的字符
        return 0;
    }

    pstr++;
    while (*pstr != ']' && *pstr != '\0' && n < MAX_LINE_SIZE) {
        str_dest[n++] = *pstr++;
    }

    str_dest[n] = '\0';
    if (*pstr != ']') {
        HCP_Logger_noid(ERR, MODULE_NAME)<<" Search ']' faild." <<HCPENDLOG;
        // 搜索']'字符时遇到非法的字符
        return 0;
    }

    return n;
}

//int CIniFile::Load(string& strConfFile, map<string, STRING_MAP>& rConfMap)
int CIniFile::Load()
{
    FILE* pFile = NULL;

    // 如果已经加载了配置文件且配置文件无变化则无需重新加载
    if (!IsModified()) {
        return SUCCESS;
    }

    string openFilePath = m_strFileName;
#ifdef WIN32
    char resolvedPath[MAX_FULL_PATH_LEN] = { 0x00 };
    if (PathCanonicalize(resolvedPath, m_strFileName.c_str()) == FALSE) {
        resolvedPath[MAX_FULL_PATH_LEN - 1] = '\0';
        openFilePath = resolvedPath;
    }
#else
    char resolvedPath[PATH_MAX + 1] = { 0x00 };
    if (realpath(m_strFileName.c_str(), resolvedPath) != NULL) {
        resolvedPath[PATH_MAX - 1] = '\0';
        openFilePath = resolvedPath;
    }
#endif
    if ((pFile = fopen(openFilePath.c_str(), "r")) == NULL) {
        HCP_Logger_noid(ERR, MODULE_NAME)<<"open file failed. path="
            << m_strFileName << ",openFilePath=" << openFilePath<<HCPENDLOG;
        return FAILED;
    }

    // 清理之前读取的配置文件缓存数据
    Clear();

    if (!ReadFileContxt(pFile)) {
        HCP_Logger_noid(ERR, MODULE_NAME)<<"Read file context failed" << HCPENDLOG;
        return FAILED;
    }
    int iRet = SUCCESS;
    iRet = CFile::GetlLastModifyTime(m_strFileName.c_str(), m_LastMTime);
    if (SUCCESS != iRet) {
        HCP_Logger_noid(ERR, MODULE_NAME)<< "Get last modify time failed."<<HCPENDLOG;
        return iRet;
    }

    return SUCCESS;
}

bool CIniFile::ReadFileContxt(FILE* pFile)
{
    char szSecName[MAX_LINE_SIZE] = {0};
    char szLineText[MAX_LINE_SIZE] = {0};
    int iSecLen = 0;
    bool bHasNextSection = false;
    CIniKeySection* pSection = NULL;
    char* pstr = ReadLine(pFile, szLineText, sizeof(szLineText));
    while (NULL != pstr && strlen(pstr) < MAX_LINE_SIZE)
    {
        while(IS_SPACE(*pstr))pstr++;

        if(!IsValidText(pstr))
        {
            pstr = ReadLine(pFile, szLineText, sizeof(szLineText));
            continue;
        }

        // 搜索SECTION
        char * dest = szSecName;
        iSecLen = CopySection(pstr,dest);
        if(0 == iSecLen)
        {
            fclose(pFile);
            return false;
        }

        pSection = new(nothrow) CIniKeySection(CMpString::ToUpper(szSecName));
        if (NULL == pSection)
        {
            HCP_Logger_noid(ERR, MODULE_NAME) << "new CIniKeySection failed" << HCPENDLOG;
            fclose(pFile);
            return false;
        }

        if (LoadSection(pFile, pSection, bHasNextSection, szLineText, MAX_LINE_SIZE)) {
            HCP_Logger_noid(ERR, MODULE_NAME)<<"Load section failed.section="
                                             << szSecName <<HCPENDLOG;
            delete pSection;
            pSection = NULL;
            fclose(pFile);
            return false;
        }

        if (!bHasNextSection)
            break;

        pstr = szLineText;
    }

    fclose(pFile);
    return true;
}

int CIniFile::WriteToFile()
{
    return SUCCESS;
}

} // namespace Module
