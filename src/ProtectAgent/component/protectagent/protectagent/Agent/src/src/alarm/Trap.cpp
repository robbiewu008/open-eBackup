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
#include "alarm/Trap.h"

#include <algorithm>

#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "common/AlarmInfoXmlParser.h"
#include "common/Path.h"
#include "message/rest/interfaces.h"

/* ------------------------------------------------------------
Function Name:SendAlarm
Description  :发送告警
Others       :------------------------------------------------------------- */
mp_int32 CTrapSender::SendAlarm(const alarm_param_t &alarmParam)
{
    LOGGUARD("");
    alarm_Info_t alarmInfo;
    mp_int32 iRet = AlarmDB::GetAlarmInfoByParam(alarmParam.iAlarmID, alarmParam.strAlarmParam, alarmInfo);
    if (alarmInfo.iAlarmSN == -1) { // 如果db中没有相同告警，新生成一条
        iRet = NewAlarmRecord(alarmParam, alarmInfo);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "NewAlarmRecord failed, alarmID = %s, alarmParam=%s",
                    alarmParam.iAlarmID.c_str(), alarmParam.strAlarmParam.c_str());
            return iRet;
        } else {
            COMMLOG(OS_LOG_INFO, "newAlarmRecord success, alarmID = %s, alarmParam=%s",
                    alarmParam.iAlarmID.c_str(), alarmParam.strAlarmParam.c_str());
        }
    }

    // 获取trap server地址信息
    std::vector<trap_server> vecServerInfo;
    iRet = AlarmDB::GetAllTrapInfo(vecServerInfo);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Get TRAP server info from database failed.");
        return iRet;
    }

    // 过滤掉已经发送过的trapserver
    std::vector<trap_server> vecTmp;
    for (mp_size i = 0; i < vecServerInfo.size(); i++) {
        if (alarmInfo.vecTrapserver.end() == std::find(
            alarmInfo.vecTrapserver.begin(), alarmInfo.vecTrapserver.end(), vecServerInfo.at(i).strServerIP)) {
            vecTmp.push_back(vecServerInfo.at(i));
        }
    }
    if (!vecTmp.empty()) {
        Pdu pdu;
        ConstructPDU(alarmInfo, pdu);
        alarmInfo.vecTrapserver = SendTrap(pdu, vecTmp);
        AlarmDB::UpdateAlarmInfo(alarmInfo);
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:SendAlarm
Description  :发送恢复告警
Others       :------------------------------------------------------------- */
mp_int32 CTrapSender::ResumeAlarm(const alarm_param_t &stAlarm)
{
    LOGGUARD("");
    alarm_Info_t alarmInfo;
    // 通过告警参数查询db中是否存在同样告警
    mp_int32 iRet = AlarmDB::GetCurrentAlarmInfoByAlarmID(stAlarm.iAlarmID, alarmInfo);
    if (alarmInfo.iAlarmSN == -1) {
        return MP_SUCCESS;
    }
    COMMLOG(OS_LOG_INFO, "ResumeAlarm, alarmID=%s, alarmParam=%s.",
        alarmInfo.iAlarmID.c_str(), alarmInfo.strAlarmParam.c_str());

    // 找到了告警记录，直接删除
    iRet = AlarmDB::DeleteAlarmInfo(alarmInfo.iAlarmSN, alarmInfo.iAlarmID);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Delete alarm failed, alarmID = %s, alarmSN = %d",
            alarmInfo.iAlarmID.c_str(), alarmInfo.iAlarmSN);
        return iRet;
    } else {
        COMMLOG(OS_LOG_INFO, "Delete alarm success, alarmID = %s, alarmSN = %d",
            alarmInfo.iAlarmID.c_str(), alarmInfo.iAlarmSN);
    }
    // trap无法取消告警，这里结束
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 发送单个Trap到Trap server
------------------------------------------------------------- */
mp_int32 CTrapSender::SendSingleTrap(Pdu &pdu, trap_server &trapServer, mp_int32 securityModel, OctetStr &securityName)
{
    // CodeDex误报,KLOCWORK.RH.LEAK
    UdpAddress address(trapServer.strServerIP.c_str());
    address.set_port((unsigned short)trapServer.iPort);
    Snmp* snmp = nullptr;
    mp_int32 status = 0;

    try {
        if (address.get_ip_version() == Address::version_ipv4) {
            if (trapServer.strListenIP.compare(UNKNOWN) == 0) {
                snmp = new Snmp(status, "0.0.0.0");
            } else { // 排除获取Agent监听IP失败的情况
                snmp = new Snmp(status, trapServer.strListenIP.c_str());
            }
        } else {
            snmp = new Snmp(status, "::");
        }
        if (snmp == NULL) {
            return MP_FAILED;
        }
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "New Snmp failed.");
        if (snmp != NULL) {
            delete snmp;
            snmp = NULL;
        }
        return MP_FAILED;
    }

    if (status != SNMP_CLASS_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Snmp Status is not success, status is %d.", status);
        delete snmp;
        snmp = NULL;
        return MP_FAILED;
    }

    UTarget utarget(address);
    utarget.set_version((snmp_version)trapServer.iVersion);  // 目前只实现SNMPV3，故写死
    utarget.set_security_model(securityModel);
    utarget.set_security_name(securityName);

    COMMLOG(OS_LOG_INFO, "Send Trap to:%s with %s.",
        trapServer.strServerIP.c_str(), trapServer.strListenIP.c_str());
    status = snmp->trap(pdu, utarget);
    if (status != SNMP_CLASS_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Snmp trap failed, status: %d, error_msg: %s.", status, Snmp::error_msg(status));
    } else {
        COMMLOG(OS_LOG_INFO, "Snmp trap success.");
    }

    if (snmp != NULL) {
        delete snmp;
        snmp = NULL;
    }

    return status;
}

/* ------------------------------------------------------------
Function Name:SendTrap
Description  :发送trap到trap server
Others       :------------------------------------------------------------- */
std::vector<mp_string> CTrapSender::SendTrap(Pdu &pdu, std::vector<trap_server> &vecServerInfo)
{
    LOGGUARD("");
    std::vector<mp_string> vecSuccessTrapserver;  // 记录发送成功的trapserver
    Snmp::socket_startup();
    snmp_v3_param stSnmpV3Param;
    CAlarmConfig::GetSnmpV3Param(stSnmpV3Param);

    // V3参数
    OctetStr privPassword(stSnmpV3Param.strPrivPassword.c_str());
    OctetStr authPassword(stSnmpV3Param.strAuthPassword.c_str());
    OctetStr securityName(stSnmpV3Param.strSecurityName.c_str());
    mp_int32 securityModel = stSnmpV3Param.iSecurityModel;
    mp_int64 authProtocol = stSnmpV3Param.iAuthProtocol;
    mp_int64 privProtocol = stSnmpV3Param.iPrivProtocol;

    v3MP* v3_MP = NULL;
    USM *usm = NULL;
    mp_string strTmpEngineId = GetLocalNodeCode();
    mp_int32 construct_status = 0;
    mp_int32 status;

    try {
        // 引擎计数器写死为1，不做文件操作处理
        mp_uint32 snmpEngineBoots = 1;
        v3_MP = new v3MP(strTmpEngineId.c_str(), snmpEngineBoots, construct_status);
        if (v3_MP == NULL) {
            (void)memset_s(&stSnmpV3Param, sizeof(stSnmpV3Param), 0, sizeof(stSnmpV3Param));
            return vecSuccessTrapserver;
        }
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "New v3MP failed.");
        goto out;
    }

    if (construct_status != SNMPv3_MP_OK) {
        COMMLOG(OS_LOG_ERROR, "construct status is not ok!, status is %d!", construct_status);
        goto out;
    }

    // 对v3_MP空指针判断不需要，coverity检查提示是死代码，屏蔽对应pclint告警。
    usm = v3_MP->get_usm();
    usm->set_discovery_mode();
    
    status = usm->add_usm_user(securityName,
        (long)authProtocol, (long)privProtocol, authPassword, privPassword);
    
    for (std::vector<trap_server>::iterator it = vecServerInfo.begin(); vecServerInfo.end() != it; ++it) {
        if (SendSingleTrap(pdu, *it, securityModel, securityName) == MP_SUCCESS) {
            vecSuccessTrapserver.push_back(it->strServerIP);
        }
    }
    Snmp::socket_cleanup();

out:
    if (v3_MP != NULL) {
        delete v3_MP;
        v3_MP = NULL;
    }
    (void)memset_s(&stSnmpV3Param, sizeof(stSnmpV3Param), 0, sizeof(stSnmpV3Param));
    return vecSuccessTrapserver;
}


RDSender RDSender::m_RDSender;
    /* ------------------------------------------------------------
Function Name:ConstructPDUCommon
Description  :根据告警信息构造pdu数据，供ConstructPDU调用
Others       :------------------------------------------------------------- */
mp_void RDSender::ConstructPDUCommon(Pdu &pdu)
{
    Vb vb;
    vb.set_oid(OID_ISM_ALARM_REPORTING_NODECODE.c_str());
    vb.set_value(GetLocalNodeCode().c_str());
    pdu += vb;
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_FAULTTYPE.c_str());
    vb.set_value(ALARM_TYPE_EQUPMENTFAULT);
    pdu += vb;
    vb.clear();
    mp_time time;
    CMpTime::Now(time);
    mp_string strNowTime = CMpTime::GetTimeString(time);
    vb.set_oid(OID_ISM_ALARM_REPORTING_FAULTTIME.c_str());
    vb.set_value(strNowTime.c_str());
    pdu += vb;
}

/* ------------------------------------------------------------
Function Name:ConstructPDU
Description  :根据告警信息构造pdu数据
Others       :------------------------------------------------------------- */
mp_bool RDSender::ConstructPDU(alarm_Info_t &stAlarm, Pdu &pdu)
{
    LOGGUARD("");
    // 华为trap上报模块OID
    Oid oid(OID_HUAWEI_TRAP_MODEL);
    pdu_security_info stPduSecurInfo;
    GetPduSecurInfo(stPduSecurInfo);

    int securityLevel = stPduSecurInfo.iSecurityLevel;
    OctetStr contextName(stPduSecurInfo.strContextName.c_str());
    OctetStr contextEngineID(stPduSecurInfo.strContextEngineID.c_str());
    Vb vb;

    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_ALARMID.c_str());
    vb.set_value(stAlarm.iAlarmID.c_str());
    pdu += vb;
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_SERIALNO.c_str());
    vb.set_value(stAlarm.iAlarmSN);
    pdu += vb;
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_ADDITIONINFO.c_str());
    vb.set_value(stAlarm.strAlarmParam.c_str());
    pdu += vb;
    vb.clear();
    vb.set_oid(OID_ISM_ALARM_REPORTING_FAULTCATEGORY.c_str());
    vb.set_value(stAlarm.iAlarmCategoryType);
    pdu += vb;
    ConstructPDUCommon(pdu);
    pdu.set_notify_id(oid);
    pdu.set_notify_enterprise(oid);
    pdu.set_security_level(securityLevel);
    pdu.set_context_name(contextName);
    pdu.set_context_engine_id(contextEngineID);
    return MP_TRUE;
}

A8000Sender A8000Sender::m_A8000Sender;

mp_bool A8000Sender::ConstructPDU(alarm_Info_t &stAlarm, Pdu &pdu)
{
    LOGGUARD("");
    // 华为trap上报模块OID
    Oid oid(OID_HUAWEI_TRAP_MODEL);
    pdu.set_notify_id(oid);
    pdu.set_notify_enterprise(oid);

    pdu_security_info stPduSecurInfo;
    GetPduSecurInfo(stPduSecurInfo);
    int securityLevel = stPduSecurInfo.iSecurityLevel;
    OctetStr contextName(stPduSecurInfo.strContextName.c_str());
    OctetStr contextEngineID(stPduSecurInfo.strContextEngineID.c_str());
    pdu.set_security_level(securityLevel);
    pdu.set_context_name(contextName);
    pdu.set_context_engine_id(contextEngineID);

    GeneratePDU(pdu, HW_STORAGE_REPORTING_ALARM_LOCATION_INFO, stAlarm.strAlarmParam);
    GeneratePDU(pdu, HW_STORAGE_REPORTING_ALARM_RESTORE_ADVICE,
        AlarmInfoXmlParser::GetInstance().GetRectification(stAlarm.iAlarmID));
    GeneratePDU(pdu, HW_STORAGE_REPORTING_ALARM_FAULT_TITLE,
        AlarmInfoXmlParser::GetInstance().GetFaultTitle(stAlarm.iAlarmID));
    GeneratePDU(pdu, HW_STORAGE_REPORTING_ALARM_FAULT_TYPE,
        AlarmInfoXmlParser::GetInstance().GetFaultType(stAlarm.iAlarmID));
    GeneratePDU(pdu, HW_STORAGE_REPORTING_ALARM_FAULT_LEVEL,
        AlarmInfoXmlParser::GetInstance().GetFaultLevel(stAlarm.iAlarmID));
    GeneratePDU(pdu, HW_STORAGE_REPORTING_ALARM_ALARM_ID, stAlarm.iAlarmID);
    GeneratePDU(pdu, HW_STORAGE_REPORTING_ALARM_FAULT_TIME, CMpTime::GetTimeString(stAlarm.strStartTime));
    GeneratePDU(pdu, HW_STORAGE_REPORTING_ALARM_SERIAL_NO, stAlarm.iAlarmSN);
    GeneratePDU(pdu, HW_STORAGE_REPORTING_ALARM_ADDITION_INFO,
        AlarmInfoXmlParser::GetInstance().GetAdditionInfo(stAlarm.iAlarmID));
    GeneratePDU(pdu, HW_STORAGE_REPORTING_ALARM_FAULT_CATEGORY,
        AlarmInfoXmlParser::GetInstance().GetFaultCategory(stAlarm.iAlarmID));
    return MP_TRUE;
}

mp_void CTrapSender::ConstructPDUCommon(Pdu &pdu)
{
    return;
}

mp_bool CTrapSender::ConstructPDU(alarm_Info_t &stAlarm, Pdu &pdu)
{
    return MP_FALSE;
}

/* ------------------------------------------------------------
Function Name:ConstructPDU
Description  :从配置文件中读取snmp相关安全配置
Others       :------------------------------------------------------------- */
mp_void CTrapSender::GetPduSecurInfo(pdu_security_info &stPduSecurInfo)
{
    LOGGUARD("");
    // 从配置文件中获取安全级别
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SNMP_SECTION, CFG_SECURITY_LEVEL,
                                                                  stPduSecurInfo.iSecurityLevel);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Get SNMP:security_level value from xml config failed");
        stPduSecurInfo.iSecurityLevel = CFG_DEFAULT_SECURITY_LEVEL;
    }
    // 从配置文件中获取context name
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, CFG_CONTEXT_NAME,
                                                          stPduSecurInfo.strContextName);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Get SNMP:context_name value from xml config failed");
        stPduSecurInfo.strContextName = "";
    }
    // 从配置文件中获取context engine id
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, CFG_ENGINE_ID,
                                                          stPduSecurInfo.strContextEngineID);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Get SNMP:engine_id value from xml config failed");
        stPduSecurInfo.strContextEngineID = "";
    }
}

/* ------------------------------------------------------------
Function Name:GetLocalNodeCode
Description  :获取本主机mac地址
Others       :------------------------------------------------------------- */
mp_string CTrapSender::GetLocalNodeCode()
{
    LOGGUARD("");
    std::vector<mp_string> vecMacs;
    mp_string strHostsnFile = CPath::GetInstance().GetConfFilePath(HOSTSN_FILE);

    mp_int32 iRet = CMpFile::ReadFile(strHostsnFile, vecMacs);
    if (iRet != MP_SUCCESS || vecMacs.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "Get host sn failed, iRet = %d, size of vecMacs is %d", iRet,
                vecMacs.size());
        return "";
    }
    mp_string code = vecMacs.front();
    code.erase(std::remove(code.begin(), code.end(), '-'), code.end());
    while (code.size() > MAXLENGTH_ENGINEID) {
        code.substr(0, code.size() - 1);
    }
    return code;
}

/* ------------------------------------------------------------
Function Name:NewAlarmRecord
Description  :新产生一条告警记录，并存入到sqlite数据库
Others       :------------------------------------------------------------- */
mp_int32 CTrapSender::NewAlarmRecord(const alarm_param_t &alarmParam, alarm_Info_t &alarmInfo)
{
    // 获取新的sn
    LOGGUARD("");
    mp_int32 iCurrSN = 0;
    mp_int32 iRet = AlarmDB::GetSN(iCurrSN);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Get CurSN Failed iRet is: %d", iRet);
        return iRet;
    }

    alarmInfo.iAlarmSN = (iCurrSN == MAX_ALARM_ID) ? 0 : (iCurrSN + 1);  // 新流水号
    alarmInfo.iAlarmID = alarmParam.iAlarmID;
    alarmInfo.strAlarmParam = alarmParam.strAlarmParam;
    alarmInfo.iAlarmType = ALARM_TYPE_EQUPMENTFAULT;
    alarmInfo.strEndTime = 0;
    CMpTime::Now(alarmInfo.strStartTime);
    alarmInfo.iAlarmCategoryType = ALARM_CATEGORY_FAULT;
    iRet = AlarmDB::InsertAlarmInfo(alarmInfo);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "InsertAlarmInfo failed");
        return iRet;
    }

    // 更新流水号
    iRet = AlarmDB::SetSN(alarmInfo.iAlarmSN);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "SetSN Failed iRet is: %d", iRet);
        return iRet;
    }

    return MP_SUCCESS;
}
