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
#include "common/ConfigXmlParse.h"
#include "common/Types.h"
#include "common/File.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"

using namespace tinyxml2;

CConfigXmlParser CConfigXmlParser::m_instance;
CConfigXmlParser& CConfigXmlParser::GetInstance()
{
    return m_instance;
}

/* ------------------------------------------------------------
Function Name:Init
Description  :初始化函数
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::Init(const mp_string& strCfgFilePath)
{
    m_strCfgFilePath = strCfgFilePath;
    m_lastTime = 0;
    return Load();
}

/* ------------------------------------------------------------
Function Name:Init
Description  :判断是否初始化函数，未初始化直接调用接口会导致进程异常退出
Others       :------------------------------------------------------------- */
mp_bool CConfigXmlParser::IsInited()
{
    return !m_strCfgFilePath.empty() ? MP_TRUE : MP_FALSE;
}

/* ------------------------------------------------------------
Function Name:Load
Description  :导入xml配置文件
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::Load()
{
    if (!CMpFile::FileExist(m_strCfgFilePath)) {
        printf("Config file is not exist, path is \"%s\".\n", BaseFileName(m_strCfgFilePath).c_str());
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    if (m_pDoc != NULL) {
        delete m_pDoc;
        m_pDoc = NULL;
    }

    try {
        // CodeDex误报，Memory Leak
        m_pDoc = new tinyxml2::XMLDocument();
    } catch (...) {
        m_pDoc = NULL;
    }

    if (!m_pDoc) {
        printf("New XMLDocument failed.\n");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    for (int i = 0; i <= COMMON_MID_RETRY_TIMES; i++) {
        if (m_pDoc->LoadFile(m_strCfgFilePath.c_str())) {
            printf("Load config xml file failed.\n");
            if (i == COMMON_MID_RETRY_TIMES) {
                printf("Load config xml file failed many times.\n");
                return ERROR_COMMON_READ_CONFIG_FAILED;
            }
            DoSleep(COMMON_MID_RETRY_INTERVAL);
        } else {
            break;
        }
    }

    mp_time tLastMoidfyTime;
    mp_int32 iRet = CMpFile::GetlLastModifyTime(m_strCfgFilePath.c_str(), tLastMoidfyTime);
    if (iRet != MP_SUCCESS) {
        printf("Get lastModifyTime of xml file failed.\n");
        return MP_FAILED;
    }
    m_lastTime = tLastMoidfyTime;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:GetChildElement
Description  :根据父节点获取子节点内容
Others       :------------------------------------------------------------- */
XMLElement* CConfigXmlParser::GetChildElement(XMLElement* pParentElement, const mp_string& strSection)
{
    if (pParentElement == NULL) {
        return NULL;
    }

    XMLElement* pCfgSec = pParentElement->FirstChildElement();
    if (pCfgSec  == NULL) {
        return NULL;
    }

    while (pCfgSec) {
        // Coverity&Fortify误报:FORTIFY.Null_Dereference
        const char* sectionName = pCfgSec->Value();
        if (sectionName == 0 || *sectionName == 0) {
            pCfgSec = pCfgSec->NextSiblingElement();
            continue;
        }

        if (strcmp(sectionName, strSection.c_str()) == 0) {
            return pCfgSec;
        } else {
            pCfgSec = pCfgSec->NextSiblingElement();
        }
    }

    return NULL;
}

// 更新文件修改时间
mp_int32 CConfigXmlParser::UpdateModifyTime()
{
    mp_time tLastMoidfyTime;
    mp_int32 iRet = CMpFile::GetlLastModifyTime(m_strCfgFilePath.c_str(), tLastMoidfyTime);
    if (iRet != MP_SUCCESS) {
        return MP_FALSE;
    }

    m_lastTime = tLastMoidfyTime;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:IsModified
Description  :判断配置文件导入后是否被修改过
Others       :------------------------------------------------------------- */
mp_bool CConfigXmlParser::IsModified()
{
    mp_int32 iRet;

    if (m_strCfgFilePath.empty()) {
        return MP_FALSE;
    }

    mp_time tLastMoidfyTime;
    iRet = CMpFile::GetlLastModifyTime(m_strCfgFilePath.c_str(), tLastMoidfyTime);
    if (iRet != MP_SUCCESS) {
        return MP_FALSE;
    }

    return (tLastMoidfyTime != m_lastTime);
}

/* ------------------------------------------------------------
Function Name:ParseNodeValue
Description  :解析xml配置某个节点的值
Others       :------------------------------------------------------------- */
mp_void CConfigXmlParser::ParseNodeValue(XMLElement* pCfgSec, NodeValue& nodeValue)
{
    if (pCfgSec == NULL) {
        return;
    }
    XMLElement* pChildItem = pCfgSec->FirstChildElement();
    if (pChildItem == NULL) {
        return;
    }

    while (pChildItem) {
        // Coverity&Fortify误报:FORTIFY.Null_Dereference
        const char* nodeName = pChildItem->Value();
        if (nodeName == NULL || *nodeName == 0) {
            pChildItem = pChildItem->NextSiblingElement();
            continue;
        }
        // 此处如果后续有多条记录的xml配置，需要重新修改
        const XMLAttribute* pAttr = pChildItem->FirstAttribute();
        if (pAttr != NULL) {
            nodeValue[nodeName] = pAttr->Value();
        }
        pChildItem = pChildItem->NextSiblingElement();
    }
}

/* ------------------------------------------------------------
Function Name:ParseNodeValue
Description  :将解析出来的值转换成字符串，只适合root下只包含一级子目录的场景
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::GetValueString(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    CThreadAutoLock lock(&m_cfgValueMutexLock);
    if (IsInited() != MP_TRUE) {
        return MP_FAILED;
    }
    if (IsModified()) {
        mp_int32 iRet = Load();
        if (iRet != MP_SUCCESS) {
            return iRet;
        }
    }

    // rootElement由tinyxml保证非空
    XMLElement* rootElement = m_pDoc->RootElement();

    XMLElement* pCfgSec = GetChildElement(rootElement, strSection);
    if (pCfgSec == NULL) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    NodeValue nodeValue;
    ParseNodeValue(pCfgSec, nodeValue);

    NodeValue::iterator itNode = nodeValue.find(strKey);
    if (itNode == nodeValue.end()) {
        printf("Section Name \"%s\" Key \"%s\" is not exist.\n", strSection.c_str(), strKey.c_str());
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    strValue = nodeValue[strKey];
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:ParseNodeValue
Description  :将解析出来的值转换成bool值，只适合root下只包含一级子目录的场景
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::GetValueBool(const mp_string& strSection, const mp_string& strKey, mp_bool& bValue)
{
    if (IsInited() != MP_TRUE) {
        return MP_FAILED;
    }
    mp_string strValue;
    mp_int32 iRet = GetValueString(strSection, strKey, strValue);
    if (iRet != MP_SUCCESS) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    mp_int32 iValue = atoi(strValue.c_str());
    if (iValue != 0) {
        bValue = MP_TRUE;
    } else {
        bValue = MP_FALSE;
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:GetValueInt32
Description  :将解析出来的值转换成int，只适合root下只包含一级子目录的场景
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::GetValueInt32(const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    if (IsInited() != MP_TRUE) {
        return MP_FAILED;
    }
    mp_string strValue;
    mp_int32 iRet = GetValueString(strSection, strKey, strValue);
    if (iRet != MP_SUCCESS) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    iValue = atoi(strValue.c_str());
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:GetValueFloat
Description  :将解析出来的值转换成float，只适合root下只包含一级子目录的场景
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::GetValueFloat(const mp_string& strSection, const mp_string& strKey, mp_float& fValue)
{
    if (IsInited() != MP_TRUE) {
        return MP_FAILED;
    }
    mp_string strValue;
    mp_int32 iRet = GetValueString(strSection, strKey, strValue);
    if (iRet != MP_SUCCESS) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    fValue = (mp_float)atof(strValue.c_str());
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:GetValueInt64
Description  :将解析出来的值转换成长int，只适合root下只包含一级子目录的场景
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::GetValueInt64(const mp_string& strSection, const mp_string& strKey, mp_int64& lValue)
{
    if (IsInited() != MP_TRUE) {
        return MP_FAILED;
    }
    mp_string strValue;
    mp_int32 iRet = GetValueString(strSection, strKey, strValue);
    if (iRet != MP_SUCCESS) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    lValue = atol(strValue.c_str());
    return MP_SUCCESS;
}

mp_int32 CConfigXmlParser::ModifyConfigFile(const mp_string& tmpFile)
{
    if (!CMpFile::FileExist(tmpFile)) {
        printf("Temp config file is not exist, path is \"%s\".\n", BaseFileName(tmpFile).c_str());
        return MP_FAILED;
    }

    tinyxml2::XMLDocument* m_pDoc2 = new tinyxml2::XMLDocument();

    if (!m_pDoc2) {
        printf("New XMLDocument failed.\n");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    for (int i = 0; i <= COMMON_MID_RETRY_TIMES; i++) {
        if (m_pDoc2->LoadFile(tmpFile.c_str())) {
            printf("Load tmp config xml file failed.\n");
            if (i == COMMON_MID_RETRY_TIMES) {
                printf("Load tmp config xml file failed many times.\n");
                return ERROR_COMMON_READ_CONFIG_FAILED;
            }
            DoSleep(COMMON_MID_RETRY_INTERVAL);
        } else {
            break;
        }
    }

    tinyxml2::XMLPrinter printer1;
    m_pDoc->Print(&printer1);
    std::string strDoc1 = printer1.CStr();

    tinyxml2::XMLPrinter printer2;
    m_pDoc2->Print(&printer2);
    std::string strDoc2 = printer2.CStr();
    // 比较写入内容一致性
    if (strDoc1 != strDoc2) {
        printf("Write temp config file faild.\n");
        return MP_FALSE;
    }

    if (CMpFile::CopyFileCoverDest(tmpFile, m_strCfgFilePath) != MP_SUCCESS) {
        printf("Copy file failed.\n");
        return MP_FAILED;
    }

    mp_int32 iRet = CMpFile::DelFile(tmpFile);
    if (iRet != MP_SUCCESS) {
        printf("Delete tmp file failed, ret is %d", iRet);
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:GetValueUint64
Description  :将解析出来的值转换成长uint，只适合root下只包含一级子目录的场景
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::GetValueUint64(const mp_string& strSection, const mp_string& strKey, mp_uint64& lValue)
{
    if (IsInited() != MP_TRUE) {
        return MP_FAILED;
    }
    mp_string strValue;
    mp_int32 iRet = GetValueString(strSection, strKey, strValue);
    if (iRet != MP_SUCCESS) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    lValue = atoll(strValue.c_str());
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:SetValue
Description  :设置配置文件某个节点的值，只适合root下只包含一级子目录的场景
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::SetValue(const mp_string& strSection, const mp_string& strKey, const mp_string& strValue)
{
    if (IsInited() != MP_TRUE) {
        return MP_FAILED;
    }
    CThreadAutoLock lock(&m_cfgValueMutexLock);
    if (!CMpFile::FileExist(m_strCfgFilePath)) {
        printf("Config file is not exist, path is \"%s\".\n", BaseFileName(m_strCfgFilePath).c_str());
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    // rootElement由tinyxml保证非空
    XMLElement* rootElement = m_pDoc->RootElement();

    XMLElement *pCfgSec = GetChildElement(rootElement, strSection);
    if (pCfgSec == NULL) {
        printf("GetChildElement failed.\n");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    // 找到Section
    XMLElement* pChildItem = pCfgSec->FirstChildElement();
    while (pChildItem) {
        // Coverity&Fortify误报:FORTIFY.Null_Dereference
        const mp_char* nodeName = pChildItem->Value();
        mp_bool bIsKeyEqual = (nodeName == NULL || *nodeName == 0 || (strcmp(nodeName, strKey.c_str()) != 0));
        if (bIsKeyEqual) {
            pChildItem = pChildItem->NextSiblingElement();
            continue;
        }
        pChildItem->SetAttribute("value", strValue.c_str());
        break;
    }
    mp_string tmpFile = m_strCfgFilePath + ".tmp";
    tinyxml2::XMLError eResult = m_pDoc->SaveFile(tmpFile.c_str());
    const char* errorStr = tinyxml2::XMLDocument::ErrorIDToName(eResult);
    mp_string errorStrValue = errorStr;
    if (eResult != tinyxml2::XML_SUCCESS) {
        printf("SaveFile failed, error is %s.\n", errorStrValue.c_str());
    }

    mp_int32 iRet = ModifyConfigFile(tmpFile);
    if (iRet != MP_SUCCESS) {
        printf("Modify config file failed.");
        return iRet;
    }

    mp_time tLastMoidfyTime;
    iRet = CMpFile::GetlLastModifyTime(m_strCfgFilePath.c_str(), tLastMoidfyTime);
    if (iRet != MP_SUCCESS) {
        return MP_FALSE;
    }

    m_lastTime = tLastMoidfyTime;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:SetValue
Description  :设置配置文件某个节点的值，只适合root下只包含2级及以上子目录的场景
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::SetValue(const mp_string& strParentSection,
    const mp_string& strChildSection, const mp_string& strKey, const mp_string& strValue)
{
    if (IsInited() != MP_TRUE) {
        return MP_FAILED;
    }
    CThreadAutoLock lockSet(&m_cfgValueMutexLock);
    if (!CMpFile::FileExist(m_strCfgFilePath)) {
        printf("Config file is not exist, path is \"%s\".\n", BaseFileName(m_strCfgFilePath).c_str());
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    // rootElement由tinyxml保证非空
    XMLElement* rootElement = m_pDoc->RootElement();

    XMLElement *pCfgParentSec = GetChildElement(rootElement, strParentSection);
    if (NULL == pCfgParentSec) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    XMLElement* pCfgSec = GetChildElement(pCfgParentSec, strChildSection);
    if (pCfgSec == NULL) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    // 找到Section
    XMLElement* pCldItem = pCfgSec->FirstChildElement();
    while (pCldItem) {
        // Coverity&Fortify误报:FORTIFY.Null_Dereference
        const mp_char* nodeName = pCldItem->Value();
        mp_bool bIsKeyEqual = (nodeName == NULL || *nodeName == 0 || (strcmp(nodeName, strKey.c_str()) != 0));
        if (bIsKeyEqual) {
            pCldItem = pCldItem->NextSiblingElement();
            continue;
        }

        pCldItem->SetAttribute("value", strValue.c_str());
        break;
    }
    mp_string tmpFile = m_strCfgFilePath + ".tmp";
    tinyxml2::XMLError eResult = m_pDoc->SaveFile(tmpFile.c_str());
    const char* errorStr = tinyxml2::XMLDocument::ErrorIDToName(eResult);
    mp_string errorStrValue = errorStr;
    if (eResult != tinyxml2::XML_SUCCESS) {
        printf("SaveFile failed, error is %s.\n", errorStrValue.c_str());
    }

    mp_int32 iRet = ModifyConfigFile(tmpFile);
    if (iRet != MP_SUCCESS) {
        printf("Modify config file failed.");
        return iRet;
    }

    mp_time tLastMoidfyTime;
    iRet = CMpFile::GetlLastModifyTime(m_strCfgFilePath.c_str(), tLastMoidfyTime);
    if (iRet != MP_SUCCESS) {
        return MP_FALSE;
    }

    m_lastTime = tLastMoidfyTime;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:GetValueString
Description  :获取配置文件某个节点的值，并转换成string，只适合root下只包含2级及以上子目录的场景
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::GetValueString(
    const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey, mp_string& strValue)
{
    if (IsInited() != MP_TRUE) {
        return MP_FAILED;
    }
    CThreadAutoLock lockGet(&m_cfgValueMutexLock);
    if (IsModified()) {
        mp_int32 iRet = Load();
        if (iRet != MP_SUCCESS) {
            return iRet;
        }
    }

    // tinyxml保证rootElement为非空
    XMLElement* rootElement = m_pDoc->RootElement();
    // CodeDex误报,KLOCWORK.NPD.FUNC.MUST
    XMLElement* pCfgParentSec = GetChildElement(rootElement, strParentSection);
    if (pCfgParentSec == NULL) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    XMLElement* pCfgSec = GetChildElement(pCfgParentSec, strChildSection);
    if (pCfgSec == NULL) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    NodeValue nodeValue;
    ParseNodeValue(pCfgSec, nodeValue);

    NodeValue::iterator itNode = nodeValue.find(strKey);
    if (itNode == nodeValue.end()) {
        printf("Parent section name \"%s\", child section name \"%s\", key \"%s\" is not exist.",
               strParentSection.c_str(),
               strChildSection.c_str(),
               strKey.c_str());
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    strValue = nodeValue[strKey];
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:GetValueBool
Description  :获取配置文件某个节点的值，并转换成bool，只适合root下只包含2级及以上子目录的场景
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::GetValueBool(
    const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey, mp_bool& bValue)
{
    if (IsInited() != MP_TRUE) {
        return MP_FAILED;
    }
    mp_string strValue;
    mp_int32 iRet = GetValueString(strParentSection, strChildSection, strKey, strValue);
    if (iRet != MP_SUCCESS) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    mp_int32 iValue = atoi(strValue.c_str());
    if (iValue != 0) {
        bValue = MP_TRUE;
    } else {
        bValue = MP_FALSE;
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:GetValueInt32
Description  :获取配置文件某个节点的值，并转换成int32，只适合root下只包含2级及以上子目录的场景
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::GetValueInt32(
    const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey, mp_int32& iValue)
{
    if (IsInited() != MP_TRUE) {
        return MP_FAILED;
    }
    mp_string strValue;
    mp_int32 iRet = GetValueString(strParentSection, strChildSection, strKey, strValue);
    if (iRet != MP_SUCCESS) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    iValue = atoi(strValue.c_str());
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:GetValueInt64
Description  :获取配置文件某个节点的值，并转换成int64，只适合root下只包含2级及以上子目录的场景
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::GetValueInt64(
    const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey, mp_int64& lValue)
{
    if (IsInited() != MP_TRUE) {
        return MP_FAILED;
    }
    mp_string strValue;
    mp_int32 iRet = GetValueString(strParentSection, strChildSection, strKey, strValue);
    if (iRet != MP_SUCCESS) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    lValue = atol(strValue.c_str());
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:GetValueFloat
Description  :获取配置文件某个节点的值，并转换成float，只适合root下只包含2级及以上子目录的场景
Others       :------------------------------------------------------------- */
mp_int32 CConfigXmlParser::GetValueFloat(
    const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey, mp_float& fValue)
{
    if (IsInited() != MP_TRUE) {
        return MP_FAILED;
    }
    mp_string strValue;
    mp_int32 iRet = GetValueString(strParentSection, strChildSection, strKey, strValue);
    if (iRet != MP_SUCCESS) {
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    fValue = (mp_float)atof(strValue.c_str());
    return MP_SUCCESS;
}
