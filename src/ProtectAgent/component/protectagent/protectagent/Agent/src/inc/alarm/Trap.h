/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Trap.h
 * @brief  Contains function declarations Trap
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef _TRAP_H_
#define _TRAP_H_

#include "common/Types.h"
#include "snmp_pp/snmp_pp.h"
#include "alarm/alarmdb.h"
#include <memory>

#define MAX_ALARM_ID          0x7FFFFFFF
#define OID_HUAWEI_TRAP_MODEL "1.3.6.1.4.1.2011.2.91.10.2.1.0.1"

 // 设备类型
enum TARGET_DEVICE { A8000 = 0, RD = 1 };

// 告警相关
#define ALARM_ID_CPUUSAGE        0x3230015 // CPU利用率告警ID
#define ALARM_ID_THAWFAILED      0x3230019 // 数据库解冻失败告警ID
#define ALARM_TYPE_EQUPMENTFAULT 2         // 设备告警

// CPU MIBOID

// 告警级别
typedef enum euAlarmLevel {
    ALARM_LEVEL_NULL,
    ALARM_LEVEL_CRITICAL,
    ALARM_LEVEL_MAJOR,
    ALARM_LEVEL_MINOR,
    ALARM_LEVEL_WARNING
} euAlarmLevel;

// 告警故障类别
typedef enum euAlarmCategory {
    ALARM_CATEGORY_NULL,   // 空
    ALARM_CATEGORY_FAULT,  // 故障告警
    ALARM_CATEGORY_RESUME  // 恢复告警
} euAlarmCategory;


typedef struct pdu_security_info_st {
    pdu_security_info_st()
    {
        iSecurityLevel = 0;
        strContextName = "";
        strContextEngineID = "";
    }
    mp_int32 iSecurityLevel;
    mp_string strContextName;
    mp_string strContextEngineID;
} pdu_security_info;

inline void GeneratePDU(Pdu &pdu, const mp_string& strOid, const mp_string& strValue)
{
    Vb vb;
    vb.set_oid(strOid.c_str());
    vb.set_value(strValue.c_str());
    pdu += vb;
}

inline void GeneratePDU(Pdu& pdu, const mp_string& strOid, mp_int32 iValue)
{
    Vb vb;
    vb.set_oid(strOid.c_str());
    vb.set_value(iValue);
    pdu += vb;
}

class CTrapSender {
public:
    mp_int32 SendAlarm(const alarm_param_t &alarmParam);
    mp_int32 ResumeAlarm(const alarm_param_t &alarmParam);
    virtual ~CTrapSender() {};
    CTrapSender() {};

protected:
    virtual mp_void ConstructPDUCommon(Pdu &pdu);
    virtual mp_bool ConstructPDU(alarm_Info_t &stAlarm, Pdu &pdu);
    static mp_int32 SendSingleTrap(Pdu &pdu, trap_server &trapServer, mp_int32 securityModel, OctetStr &securityName);
    static std::vector<mp_string> SendTrap(Pdu &pdu, std::vector<trap_server> &vecServerInfo);
    static mp_void GetPduSecurInfo(pdu_security_info &stPduSecurInfo);
    static mp_string GetLocalNodeCode();
    static mp_int32 NewAlarmRecord(const alarm_param_t &alarmParam, alarm_Info_t &alarmInfo);
};

class A8000Sender : public CTrapSender {
public:
    virtual ~A8000Sender() {};
    static A8000Sender& GetInstance()
    {
        return m_A8000Sender;
    }

protected:
    mp_bool ConstructPDU(alarm_Info_t &stAlarm, Pdu &pdu);

private:
    A8000Sender() {};
    static A8000Sender m_A8000Sender;
};

class RDSender : public CTrapSender {
public:
    virtual ~RDSender() {};
    static RDSender& GetInstance()
    {
        return m_RDSender;
    }

protected:
    mp_void ConstructPDUCommon(Pdu &pdu);
    mp_bool ConstructPDU(alarm_Info_t &stAlarm, Pdu &pdu);

private:
    RDSender() {};
    static RDSender m_RDSender;
};

class CTrapSenderManager {
public:
    static CTrapSender &CreateSender(TARGET_DEVICE target)
    {
        if (target == A8000) {
            return A8000Sender::GetInstance();
        } else {
            return RDSender::GetInstance();
        }
    }
};

#endif

