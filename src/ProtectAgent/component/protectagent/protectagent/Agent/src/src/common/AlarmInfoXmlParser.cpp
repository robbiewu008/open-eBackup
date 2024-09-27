#include "common/AlarmInfoXmlParser.h"
#include "common/Types.h"
#include "common/File.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/Log.h"

using namespace tinyxml2;

AlarmInfoXmlParser AlarmInfoXmlParser::m_instance;

/* ------------------------------------------------------------
Function Name:Init
Description  :初始化函数
Others       :------------------------------------------------------------- */
mp_int32 AlarmInfoXmlParser::Init(const mp_string& strCfgFilePath)
{
    m_strCfgFilePath = strCfgFilePath;
    m_lastTime = 0;
    return Load();
}

/* ------------------------------------------------------------
Function Name:Load
Description  :导入xml配置文件
Others       :------------------------------------------------------------- */
mp_int32 AlarmInfoXmlParser::Load()
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
    if (m_pDoc->LoadFile(m_strCfgFilePath.c_str())) {
        printf("Load config xml file failed.\n");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

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
mp_bool AlarmInfoXmlParser::IsModified()
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


tinyxml2::XMLElement* AlarmInfoXmlParser::GetAlarmElement(const mp_string& strAlarmID)
{
    CThreadAutoLock lock(&m_cfgValueMutexLock);
    if (IsModified()) {
        mp_int32 iRet = Load();
        if (iRet != MP_SUCCESS) {
            return NULL;
        }
    }
    // rootElement由tinyxml保证非空
    XMLElement* rootElement = m_pDoc->RootElement();
    if (rootElement == NULL) {
        return NULL;
    }
    XMLElement* pElemAlarm = rootElement->FirstChildElement("Alarm");
    while (pElemAlarm != NULL) {
        const XMLAttribute* pAtt = pElemAlarm->FindAttribute("id");
        if (pAtt != NULL) {
            if (pAtt->Value() == strAlarmID) {
                break;
            }
        }
        pElemAlarm = pElemAlarm->NextSiblingElement();
    }

    return pElemAlarm;
}

mp_string AlarmInfoXmlParser::GetRectification(const mp_string& strAlarmID)
{
    mp_string ret;

    XMLElement* pElemAlarm = GetAlarmElement(strAlarmID);
    if (pElemAlarm == NULL) {
        return ret;
    }
    XMLElement* pNode = pElemAlarm->FirstChildElement("Rectification");
    if (pNode == NULL) {
        return ret;
    }
    const XMLAttribute* pAtt = pNode->FindAttribute("en_US");
    if (pAtt != NULL) {
        ret = pAtt->Value();
    }
    return ret;
}

mp_string AlarmInfoXmlParser::GetFaultTitle(const mp_string& strAlarmID)
{
    mp_string ret;

    XMLElement* pElemAlarm = GetAlarmElement(strAlarmID);
    if (pElemAlarm == NULL) {
        return ret;
    }
    XMLElement* pNode = pElemAlarm->FirstChildElement("FaultTitle");
    if (pNode == NULL) {
        return ret;
    }
    const XMLAttribute* pAtt = pNode->FindAttribute("en_US");
    if (pAtt != NULL) {
        ret = pAtt->Value();
    }
    return ret;
}

mp_int32 AlarmInfoXmlParser::GetFaultType(const mp_string& strAlarmID)
{
    mp_int32 ret = 0;

    XMLElement* pElemAlarm = GetAlarmElement(strAlarmID);
    if (pElemAlarm == NULL) {
        return ret;
    }
    XMLElement* pNode = pElemAlarm->FirstChildElement("FaultType");
    if (pNode == NULL) {
        return ret;
    }
    const XMLAttribute* pAtt = pNode->FindAttribute("value");
    if (pAtt != NULL) {
        ret = atoi(pAtt->Value());
    }
    return ret;
}

mp_int32 AlarmInfoXmlParser::GetFaultLevel(const mp_string& strAlarmID)
{
    mp_int32 ret = 0;

    XMLElement* pElemAlarm = GetAlarmElement(strAlarmID);
    if (pElemAlarm == NULL) {
        return ret;
    }
    XMLElement* pNode = pElemAlarm->FirstChildElement("FaultLevel");
    if (pNode == NULL) {
        return ret;
    }
    const XMLAttribute* pAtt = pNode->FindAttribute("value");
    if (pAtt != NULL) {
        ret = atoi(pAtt->Value());
    }
    return ret;
}

mp_string AlarmInfoXmlParser::GetAdditionInfo(const mp_string& strAlarmID)
{
    mp_string ret;

    XMLElement* pElemAlarm = GetAlarmElement(strAlarmID);
    if (pElemAlarm == NULL) {
        return ret;
    }
    XMLElement* pNode = pElemAlarm->FirstChildElement("AdditionInfo");
    if (pNode == NULL) {
        return ret;
    }
    const XMLAttribute* pAtt = pNode->FindAttribute("en_US");
    if (pAtt != NULL) {
        ret = pAtt->Value();
    }
    return ret;
}

mp_int32 AlarmInfoXmlParser::GetFaultCategory(const mp_string& strAlarmID)
{
    mp_int32 ret = 0;

    XMLElement* pElemAlarm = GetAlarmElement(strAlarmID);
    if (pElemAlarm == NULL) {
        return ret;
    }
    XMLElement* pNode = pElemAlarm->FirstChildElement("FaultCategory");
    if (pNode == NULL) {
        return ret;
    }
    const XMLAttribute* pAtt = pNode->FindAttribute("value");
    if (pAtt != NULL) {
        ret = atoi(pAtt->Value());
    }
    return ret;
}
