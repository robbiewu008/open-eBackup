/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file alarmdb.h
 * @brief  Contains function declarations alarmdb
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
 */
#ifndef _ALARMDB_H_
#define _ALARMDB_H_

#include "common/Types.h"
#include <vector>

static const mp_uchar DEFAULT_PORT_NUM = 162;
static const mp_uchar DEFAULT_VERSION = 3;
#define MAX_ALARM_ID 0x7FFFFFFF

// SNMP协议类型
typedef enum { SNMP_V1 = 0, SNMP_V2C, SNMP_V3 = 3 } SNMP_TYPE;

inline SNMP_TYPE TrSNMPVersion(const mp_string& str)
{
    if (str == "V3") {
        return SNMP_V3;
    } else if (str == "V2C") {
        return SNMP_V2C;
    } else {
        return SNMP_V1;
    }
}

typedef enum {
    AUTH_PROTOCOL_NONE = 1,
    AUTH_PROTOCOL_MD5,
    AUTH_PROTOCOL_SHA1,
    AUTH_PROTOCOL_SHA2 = 5
} AUTH_PROTOCOL_TYPE;

typedef enum {
    ALARM,
    EVENT = 5,
} ALARM_CLASS;

inline AUTH_PROTOCOL_TYPE TrAuthProtocol(const mp_string& str)
{
    if (str == "HMACMD5") {
        return AUTH_PROTOCOL_MD5;
    } else if (str == "HMAC_SHA1") {
        return AUTH_PROTOCOL_SHA1;
    } else if (str == "HMAC_SHA2") {
        return AUTH_PROTOCOL_SHA2;
    } else {
        return AUTH_PROTOCOL_NONE;
    }
}

typedef enum { PRIVATE_PROTOCOL_NONE = 1, PRIVATE_PROTOCOL_DES, PRIVATE_PROTOCOL_AES128 = 4 } PRIVATE_PROTOCOL_TYPE;

inline PRIVATE_PROTOCOL_TYPE TrPrivateProtocol(const mp_string& str)
{
    if (str == "DES") {
        return PRIVATE_PROTOCOL_DES;
    } else if (str == "AES") {
        return PRIVATE_PROTOCOL_AES128;
    } else {
        return PRIVATE_PROTOCOL_NONE;
    }
}

typedef enum { SECURITY_LEVEL_NOAUTH_NOPRIV = 1, SECURITY_LEVEL_NOPRI, SECURITY_LEVEL_AUTH_PRIV } SECURITY_LEVEL;

inline SECURITY_LEVEL TrSecurityLevel(AUTH_PROTOCOL_TYPE iAuthProtocol, PRIVATE_PROTOCOL_TYPE iPrivProtocol)
{
    if (iAuthProtocol == AUTH_PROTOCOL_NONE) {
        return SECURITY_LEVEL_NOAUTH_NOPRIV;
    } else if (iPrivProtocol == PRIVATE_PROTOCOL_NONE) {
        return SECURITY_LEVEL_NOPRI;
    } else {
        return SECURITY_LEVEL_AUTH_PRIV;
    }
}

typedef enum { SECURITY_MODEL_ANY = 0, SECURITY_MODEL_V1, SECURITY_MODEL_V2, SECURITY_MODEL_USM } SECURITY_MODEL;

inline SECURITY_MODEL TrSecurityModel(SNMP_TYPE version)
{
    if (version == SNMP_V3) {
        return SECURITY_MODEL_USM;
    } else if (version == SNMP_V2C) {
        return SECURITY_MODEL_V2;
    } else if (version == SNMP_V1) {
        return SECURITY_MODEL_V1;
    } else {
        return SECURITY_MODEL_ANY;
    }
}

typedef struct alarm_Info_st {
    mp_int32 iAlarmSN;
    mp_string iAlarmID;
    mp_int32 iAlarmType;
    mp_int32 iAlarmCategoryType;
    mp_int32 iAlarmClass;
    bool isSuccess;
    mp_time strStartTime;
    mp_time strEndTime;
    mp_string strAlarmParam;
    mp_string sourceType;
    mp_int32 severity = -1;
    std::vector<mp_string> vecTrapserver;
    mp_string resourceId;
    alarm_Info_st()
    {
        iAlarmSN = -1;
        iAlarmType = 0;
        iAlarmCategoryType = 0;
        iAlarmClass = ALARM_CLASS::ALARM;
        isSuccess = true;
    }
} alarm_Info_t;

// 发送告警的必须参数
typedef struct alarm_param_st {
    mp_string iAlarmID;
    mp_string strAlarmParam;
    mp_int32 iAlarmClass = ALARM_CLASS::ALARM;
    bool isSuccess = true;
    mp_string sourceType;
    mp_int32 alarmType = -1;
    mp_int32 severity = -1;
    mp_int64 sequence = -1;
    mp_string resourceId;
} alarm_param_t;

typedef struct trap_server_st {
    mp_int32 iPort;
    mp_int32 iVersion;
    mp_string strServerIP;
    mp_string strListenIP;  // 本机监听IP，当本机存在多个IP时，注册TRAP时使用注册时的IP，防止trap收不到信息
} trap_server;

typedef struct snmp_v3_param_st {
    mp_string strContextEngineId;
    mp_string strContextName;
    mp_string strSecurityName;
    mp_int32 iSecurityModel;
    mp_int32 iSecurityLevel;
    mp_string strAuthPassword;
    mp_string strPrivPassword;
    mp_int32 iAuthProtocol;
    mp_int32 iPrivProtocol;
    snmp_v3_param_st()
    {
        iSecurityModel = SECURITY_MODEL_ANY;
        iSecurityLevel = SECURITY_LEVEL_NOAUTH_NOPRIV;
        iAuthProtocol = AUTH_PROTOCOL_NONE;
        iPrivProtocol = PRIVATE_PROTOCOL_NONE;
    }
} snmp_v3_param;

class AlarmDB {
public:
    static mp_int32 InsertAlarmInfo(alarm_Info_t& stAlarmInfo);
    static mp_int32 DeleteAlarmInfo(mp_int32 iAlarmSN, const mp_string& iAlarmID);
    static mp_int32 UpdateAlarmInfo(alarm_Info_t& stAlarmInfo);
    static mp_int32 GetAllAlarmInfo(std::vector<alarm_Info_t>& vecAlarmInfo);
    static mp_int32 GetAlarmInfoBySNAndID(mp_int32 iAlarmSN, const mp_string& iAlarmID, alarm_Info_t& stAlarmInfo);
    static mp_int32 GetCurrentAlarmInfoByAlarmID(const mp_string& strAlarmID, alarm_Info_t& stAlarmInfo);
    static mp_int32 GetAlarmInfoByParam(const mp_string& iAlarmID, const mp_string& strAlarmParam,
        alarm_Info_t& stAlarmInfo);
    static mp_int32 GetSN(mp_int32& iAlarmSn);
    static mp_int32 SetSN(mp_int32 iAlarmSn);
    static mp_int32 InsertTrapServer(trap_server& stTrapServer);
    static mp_int32 DeleteTrapServer(trap_server& stTrapServer);
    static mp_int32 UpdateAllTrapInfo(const std::vector<trap_server>& vecStServerInfo);
    static mp_int32 GetAllTrapInfo(std::vector<trap_server>& vecStServerInfo);
    static mp_int32 DeleteAllTrapServer();

private:
    static mp_bool BeExistInTrapInfo(trap_server& stTrapServer);
    static mp_int32 CheckTrapInfoTable();
};

class CAlarmConfig {
public:
    static mp_int32 UpdateSnmpV3Param(const snmp_v3_param& stSnmpV3Param);
    static mp_void GetSnmpV3Param(snmp_v3_param& stSnmpV3Param);
};

#endif
