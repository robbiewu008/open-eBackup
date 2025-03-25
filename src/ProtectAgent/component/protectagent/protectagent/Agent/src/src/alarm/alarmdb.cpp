/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file alarmdb.cpp
 * @brief  The implemention about alarmdb
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "common/DB.h"
#include "common/MpString.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "securecom/CryptAlg.h"
#include "alarm/alarmdb.h"
using namespace std;
/* ------------------------------------------------------------
Function Name: InsertAlarmInfo
Description  : 将alarm信息插入到sqlite数据库
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::InsertAlarmInfo(alarm_Info_t& stAlarmInfo)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff << "insert into " << ALARM_TABLE << "(" << TITLE_ALARM_SERIALNO << "," << TITLE_ALARM_ID << ","
         << TITLE_ALARM_LEVEL << "," << TITLE_ALARM_TYPE << "," << TITLE_ALARM_CATEGORY << "," << TITLE_START_TIME
         << "," << TITLE_END_TIME << "," << TITLE_ALARM_PARAM << "," << TITLE_TRAPSERVER
         << ") values(?,?,?,?,?,?,?,?,?);";

    DbParamStream dps;
    DbParam dp = stAlarmInfo.iAlarmSN;
    dps << std::move(dp);
    dp = stAlarmInfo.iAlarmID;
    dps << std::move(dp);
    dp = stAlarmInfo.severity;
    dps << std::move(dp);
    dp = stAlarmInfo.iAlarmType;
    dps << std::move(dp);
    dp = stAlarmInfo.iAlarmCategoryType;
    dps << std::move(dp);
    dp = mp_int64(stAlarmInfo.strStartTime);
    dps << std::move(dp);
    dp = mp_int64(stAlarmInfo.strEndTime);
    dps << std::move(dp);
    dp = stAlarmInfo.strAlarmParam;
    dps << std::move(dp);
    dp = CMpString::StrJoin(stAlarmInfo.vecTrapserver, STR_COMMA);
    dps << std::move(dp);

    mp_string sql = buff.str();
    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.ExecSql failed,iRet = %d.", iRet);
    }

    return iRet;
}

/* ------------------------------------------------------------
Function Name: DeleteAlarmInfo
Description  : 删除sqlite数据库中某条告警数据
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::DeleteAlarmInfo(mp_int32 iAlarmSN, const mp_string& iAlarmID)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff << "delete from " << ALARM_TABLE << " where " << TITLE_ALARM_SERIALNO << " == ?"
         << " and " << TITLE_ALARM_ID << " == ?;";

    DbParamStream dps;
    DbParam dp = iAlarmSN;
    dps << std::move(dp);
    dp = iAlarmID;
    dps << std::move(dp);

    mp_string sql = buff.str();
    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.ExecSql failed,iRet = %d.", iRet);
    }
    return iRet;
}

/* ------------------------------------------------------------
Function Name: UpdateAlarmInfo
Description  : 更新sqlite数据库中某条告警数据
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::UpdateAlarmInfo(alarm_Info_t& stAlarmInfo)
{
    LOGGUARD("");
    ostringstream buff;
    buff << "update " << ALARM_TABLE
        << " set " << TITLE_ALARM_LEVEL << "= ?," << TITLE_ALARM_TYPE << "= ?," << TITLE_ALARM_CATEGORY
        << "= ?," << TITLE_START_TIME << "= ?," << TITLE_END_TIME << "= ?,"
        << TITLE_ALARM_PARAM << "= ?," << TITLE_TRAPSERVER << "= ?"
        << " where " << TITLE_ALARM_SERIALNO << " == ? and " << TITLE_ALARM_ID << " == ?;";

    DbParamStream dps;
    DbParam dp = stAlarmInfo.severity;
    dps << std::move(dp);
    dp = stAlarmInfo.iAlarmType;
    dps << std::move(dp);
    dp = stAlarmInfo.iAlarmCategoryType;
    dps << std::move(dp);
    dp = mp_int64(stAlarmInfo.strStartTime);
    dps << std::move(dp);
    dp = mp_int64(stAlarmInfo.strEndTime);
    dps << std::move(dp);
    dp = stAlarmInfo.strAlarmParam;
    dps << std::move(dp);
    dp = CMpString::StrJoin(stAlarmInfo.vecTrapserver, STR_COMMA);
    dps << std::move(dp);
    dp = stAlarmInfo.iAlarmSN;
    dps << std::move(dp);
    dp = stAlarmInfo.iAlarmID;
    dps << std::move(dp);

    mp_string sql = buff.str();
    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.ExecSql failed,iRet = %d.", iRet);
    }
    return iRet;
}

/* ------------------------------------------------------------
Function Name: UpdateAlarmInfo
Description  : 查询sqlite数据库中所有告警数据
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::GetAllAlarmInfo(vector<alarm_Info_t>& vecAlarmInfo)
{
    LOGGUARD("");
    ostringstream buff;
    buff << "select * from " << ALARM_TABLE;
    mp_string sql = buff.str();

    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    DbParamStream dps;
    mp_int32 iRet = CDB::GetInstance().QueryTable(sql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.QueryTable failed, iRet = %d.", iRet);
        return iRet;
    }

    alarm_Info_t stAlarmInfo;
    mp_string strTmp;
    vecAlarmInfo.reserve(iRowCount);
    for (mp_int32 iRow = 1; iRow <= iRowCount; ++iRow) {
        readBuff >> strTmp;
        if (strTmp.empty()) {
            stAlarmInfo.iAlarmSN = -1;
        } else {
            stAlarmInfo.iAlarmSN = atoint32(strTmp.c_str());
        }
        readBuff >> strTmp;
        stAlarmInfo.iAlarmID = strTmp;
        readBuff >> strTmp;
        stAlarmInfo.severity = atoint32(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.iAlarmType = atoint32(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.iAlarmCategoryType = atoint32(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.strStartTime = atoint64(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.strEndTime = atoint64(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.strAlarmParam = strTmp;
        readBuff >> strTmp;
        CMpString::StrSplit(stAlarmInfo.vecTrapserver, strTmp, STR_COMMA.at(0));
        vecAlarmInfo.emplace_back(stAlarmInfo);
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GetAlarmInfoBySNAndID
Description  : 根据sn和id获取sqlite数据中某条告警数据
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::GetAlarmInfoBySNAndID(mp_int32 iAlarmSN, const mp_string& iAlarmID, alarm_Info_t& stAlarmInfo)
{
    LOGGUARD("");
    ostringstream buff;
    buff << "select * from " << ALARM_TABLE << " where " << TITLE_ALARM_SERIALNO
    << " == ? and " << TITLE_ALARM_ID << " == ?;";

    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    DbParamStream dps;
    DbParam dp = iAlarmSN;
    dps << std::move(dp);
    dp = iAlarmID;
    dps << std::move(dp);

    mp_string sql = buff.str();
    mp_int32 iRet = CDB::GetInstance().QueryTable(sql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.QueryTable failed, iRet = %d.", iRet);
        return iRet;
    }

    mp_string strTmp;
    for (mp_int32 iRow = 1; iRow <= iRowCount; ++iRow) {
        readBuff >> strTmp;
        if (strTmp.empty()) {
            stAlarmInfo.iAlarmSN = -1;
        } else {
            stAlarmInfo.iAlarmSN = atoint32(strTmp.c_str());
        }
        readBuff >> strTmp;
        stAlarmInfo.iAlarmID = strTmp;
        readBuff >> strTmp;
        stAlarmInfo.severity = atoint32(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.iAlarmType = atoint32(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.iAlarmCategoryType = atoint32(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.strStartTime = atoint64(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.strEndTime = atoint64(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.strAlarmParam = strTmp;
        readBuff >> strTmp;
        CMpString::StrSplit(stAlarmInfo.vecTrapserver, strTmp, STR_COMMA.at(0));
        break;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GetCurrentAlarmInfoByAlarmID
Description  : 根据sn和id获取sqlite数据中某条当前告警数据
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::GetCurrentAlarmInfoByAlarmID(const mp_string& strAlarmID, alarm_Info_t& stAlarmInfo)
{
    LOGGUARD("");
    ostringstream buff;
    buff << "select * from " << ALARM_TABLE << " where " << TITLE_ALARM_ID << " == ?;";

    mp_string sql = buff.str();

    DbParamStream dps;
    DbParam dp = strAlarmID;
    dps << std::move(dp);

    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    mp_int32 iRet = CDB::GetInstance().QueryTable(sql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.QueryTable failed, iRet = %d.", iRet);
        return iRet;
    }
    
    mp_string strTmp;
    for (mp_int32 iRow = 1; iRow <= iRowCount; ++iRow) {
        readBuff >> strTmp;
        if (strTmp.empty()) {
            stAlarmInfo.iAlarmSN = -1;
        } else {
            stAlarmInfo.iAlarmSN = atoint32(strTmp.c_str());
        }
        readBuff >> strTmp;
        stAlarmInfo.iAlarmID = strTmp;
        readBuff >> strTmp;
        stAlarmInfo.severity = atoint32(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.iAlarmType = atoint32(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.iAlarmCategoryType = atoint32(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.strStartTime = atoint64(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.strEndTime = atoint64(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.strAlarmParam = strTmp;
        readBuff >> strTmp;
        CMpString::StrSplit(stAlarmInfo.vecTrapserver, strTmp, STR_COMMA.at(0));
        break;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GetAlarmInfoByParam
Description  : 根据告警参数获取sqlite数据中某条告警数据
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::GetAlarmInfoByParam(const mp_string& iAlarmID, const mp_string& strAlarmParam,
    alarm_Info_t& stAlarmInfo)
{
    LOGGUARD("");
    ostringstream buff;
    buff << "select * from " << ALARM_TABLE << " where " << TITLE_ALARM_PARAM
    << " == ? and " << TITLE_ALARM_ID << " == ?;";

    DbParamStream dps;
    DbParam dp = strAlarmParam;
    dps << std::move(dp);
    dp = iAlarmID;
    dps << std::move(dp);

    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    mp_string sql = buff.str();
    mp_int32 iRet = CDB::GetInstance().QueryTable(sql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.QueryTable failed, iRet = %d.", iRet);
        return iRet;
    }

    mp_string strTmp;
    for (mp_int32 iRow = 1; iRow <= iRowCount; ++iRow) {
        readBuff >> strTmp;
        if (strTmp.empty()) {
            stAlarmInfo.iAlarmSN = -1;
        } else {
            stAlarmInfo.iAlarmSN = atoint32(strTmp.c_str());
        }
        readBuff >> strTmp;
        stAlarmInfo.iAlarmID = strTmp;
        readBuff >> strTmp;
        stAlarmInfo.severity = atoint32(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.iAlarmType = atoint32(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.iAlarmCategoryType = atoint32(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.strStartTime = atoint64(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.strEndTime = atoint64(strTmp.c_str());
        readBuff >> strTmp;
        stAlarmInfo.strAlarmParam = strTmp;
        readBuff >> strTmp;
        CMpString::StrSplit(stAlarmInfo.vecTrapserver, strTmp, STR_COMMA.at(0));
        break;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GetSN
Description  : 获取告警流水号
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::GetSN(mp_int32& iAlarmSn)
{
    LOGGUARD("");
    ostringstream buff;
    buff << "select " << TITLE_ALARM_SN << " from " << ALARM_TYPE_TABLE;

    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
    DbParamStream dps;
    mp_string sql = buff.str();
    mp_int32 iRet = CDB::GetInstance().QueryTable(sql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.QueryTable failed, iRet = %d.", iRet);
        return iRet;
    }

    mp_string strAlarmSN = "";
    if (iRowCount >= 1) {
        readBuff >> strAlarmSN;
        iAlarmSn = atoi(strAlarmSN.c_str());
    } else {
        // 如果数据库中没有，则插入一个alarmID的条目，SN从0开始计数
        buff.str("");
        buff << "insert into " << ALARM_TYPE_TABLE << "( " << TITLE_ALARM_SN << ") values(?);";
        dps.Clear();
        DbParam dp = 0;
        dps << std::move(dp);
        iRet = CDB::GetInstance().ExecSql(buff.str(), dps);
        if (iRet != 0) {
            COMMLOG(OS_LOG_ERROR, "db.ExecSql failed,iRet = %d.", iRet);
            return iRet;
        }
        iAlarmSn = 0;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: SetSN
Description  : 将告警流水号存入sqlite数据库
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::SetSN(mp_int32 iAlarmSn)
{
    LOGGUARD("");
    ostringstream buff;
    buff << "select " << TITLE_ALARM_SN << " from " << ALARM_TYPE_TABLE;

    DbParamStream dps;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    mp_int32 iRet = CDB::GetInstance().QueryTable(buff.str(), dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.QueryTable failed, iRet = %d.", iRet);
        return iRet;
    }

    if (iRowCount >= 1) {
        COMMLOG(OS_LOG_DEBUG, "find record in AlarmType table");
        buff.str("");
        buff << "update " << ALARM_TYPE_TABLE << " set " << TITLE_ALARM_SN << " = ?;";
        mp_string sql = buff.str();
        COMMLOG(OS_LOG_DEBUG, "sql:%s", sql.c_str());

        dps.Clear();
        DbParam dp = iAlarmSn;
        dps << std::move(dp);
        iRet = CDB::GetInstance().ExecSql(sql, dps);
        if (iRet != 0) {
            COMMLOG(OS_LOG_ERROR, "db.ExecSql failed, iRet = %d.", iRet);
            return iRet;
        }
    } else {
        COMMLOG(OS_LOG_DEBUG, "can not find record in AlarmType table");
        // 如果数据库中没有，则插入一个alarmID的条目
        buff.str("");
        buff << "insert into " << ALARM_TYPE_TABLE << "( " << TITLE_ALARM_SN << ") values(?);";
        dps.Clear();
        DbParam dp = iAlarmSn;
        dps << std::move(dp);
        mp_string sql = buff.str();
        iRet = CDB::GetInstance().ExecSql(sql, dps);
        if (iRet != 0) {
            COMMLOG(OS_LOG_ERROR, "db.ExecSql failed, iRet = %d.", iRet);
            return iRet;
        }
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: InsertTrapServer
Description  : 在数据库中插入一条trap server信息
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::InsertTrapServer(trap_server& stTrapServer)
{
    LOGGUARD("");
    mp_int32 iRet = CheckTrapInfoTable();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CheckTrapInfoTable failed, iRet = %d.", iRet);
        return iRet;
    }

    if (BeExistInTrapInfo(stTrapServer)) {
        // 已经存在，打印日志
        COMMLOG(OS_LOG_INFO, "%s is exist", stTrapServer.strServerIP.c_str());
        return MP_SUCCESS;
    }

    ostringstream buff;
    buff << "insert into " << TRAP_SERVER_TABLE << "(TrapServerIP, TrapPort, SnmpVersion, AgentIP) values(?, ?, ?, ?);";
    mp_string sql = buff.str();
    
    DbParamStream dps;
    DbParam dp = stTrapServer.strServerIP;
    dps << std::move(dp);
    dp = stTrapServer.iPort;
    dps << std::move(dp);
    dp = stTrapServer.iVersion;
    dps << std::move(dp);
    dp = stTrapServer.strListenIP;
    dps << std::move(dp);
    iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.ExecSql failed,iRet = %d.", iRet);
    }
    return iRet;
}

/* ------------------------------------------------------------
Function Name: DeleteTrapServer
Description  : 在数据库中删除一条trap server信息
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::DeleteTrapServer(trap_server& stTrapServer)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff << "delete from " << TRAP_SERVER_TABLE << " where TrapServerIP == ? and TrapPort == ? and SnmpVersion == ?;";
    mp_string sql = buff.str();

    DbParamStream dps;
    DbParam dp = stTrapServer.strServerIP;
    dps << std::move(dp);
    dp = stTrapServer.iPort;
    dps << std::move(dp);
    dp = stTrapServer.iVersion;
    dps << std::move(dp);

    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.ExecSql failed,iRet = %d.", iRet);
    }
    return iRet;
}

/* ------------------------------------------------------------
Function Name: UpdateAllTrapInfo
Description  : 更新数据中所有的trap server信息
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::UpdateAllTrapInfo(const std::vector<trap_server>& vecStServerInfo)
{
    LOGGUARD("");
    mp_int32 iRet = CheckTrapInfoTable();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CheckTrapInfoTable failed, iRet = %d.", iRet);
        return iRet;
    }

    iRet = DeleteAllTrapServer();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "DeleteAllTrapServer failed, iRet = %d.", iRet);
        return iRet;
    }

    for (vector<trap_server>::const_iterator iter = vecStServerInfo.begin(); iter != vecStServerInfo.end(); ++iter) {
        ostringstream buff;
        buff << "insert into " << TRAP_SERVER_TABLE
             << "(TrapServerIP, TrapPort, SnmpVersion, AgentIP) values(?, ?, ?, ?);";
        DbParamStream dps;
        DbParam dp = (*iter).strServerIP;
        dps << std::move(dp);
        dp = (*iter).iPort;
        dps << std::move(dp);
        dp = (*iter).iVersion;
        dps << std::move(dp);
        dp = (*iter).strListenIP;
        dps << std::move(dp);
        iRet = CDB::GetInstance().ExecSql(buff.str(), dps);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "db.ExecSql failed,iRet = %d.", iRet);
            break;
        }
    }
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "insert trap_server faild, DeleteAllTrapServer");
        DeleteAllTrapServer();
    }
    return iRet;
}

/* ------------------------------------------------------------
Function Name: GetAllTrapInfo
Description  : 查询数据中所有的trap server信息
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::GetAllTrapInfo(vector<trap_server>& vecStServerInfo)
{
    LOGGUARD("");
    ostringstream buff;
    buff << "select TrapServerIP,TrapPort,SnmpVersion,AgentIP from " << TRAP_SERVER_TABLE;

    mp_int32 iRet = CheckTrapInfoTable();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CheckTrapInfoTable failed, iRet = %d.", iRet);
        return iRet;
    }

    DbParamStream dps;
    DBReader readBuff;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    iRet = CDB::GetInstance().QueryTable(buff.str(), dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.QueryTable failed, iRet = %d.", iRet);
        return iRet;
    }

    vecStServerInfo.reserve(iRowCount);
    for (mp_int32 iRow = 1; iRow <= iRowCount; ++iRow) {
        trap_server stServerInfo;
        mp_string strServerIP = "";
        mp_string strPort = "";
        mp_string strVersion = "";
        mp_string strListenIP = "";
        readBuff >> strServerIP;
        readBuff >> strPort;
        readBuff >> strVersion;
        readBuff >> strListenIP;
        stServerInfo.strServerIP = std::move(strServerIP);
        stServerInfo.iPort = atoi(strPort.c_str());
        stServerInfo.iVersion = atoi(strVersion.c_str());
        stServerInfo.strListenIP = std::move(strListenIP);
        vecStServerInfo.emplace_back(stServerInfo);
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: DeleteAllTrapServer
Description  : 删除数据中所有的trap server信息
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::DeleteAllTrapServer()
{
    LOGGUARD("");
    std::ostringstream buff;
    buff << "delete from " << TRAP_SERVER_TABLE;

    mp_string sql = buff.str();
    DbParamStream dps;
    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.ExecSql failed,iRet = %d.", iRet);
    }
    return iRet;
}

/* ------------------------------------------------------------
Function Name: GetAllTrapInfo
Description  : 查询数据中是否存在指定的trap server信息
Others       :-------------------------------------------------------- */
mp_bool AlarmDB::BeExistInTrapInfo(trap_server& stTrapServer)
{
    LOGGUARD("");
    ostringstream buff;
    buff << "select TrapServerIP,TrapPort,SnmpVersion from " << TRAP_SERVER_TABLE
        << " where TrapServerIP == ? and TrapPort == ? and SnmpVersion == ?;";

    DbParamStream dps;
    DbParam dp = stTrapServer.strServerIP;
    dps << std::move(dp);
    dp = stTrapServer.iPort;
    dps << std::move(dp);
    dp = stTrapServer.iVersion;
    dps << std::move(dp);

    mp_string strSql = buff.str();
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    mp_int32 iRet = CDB::GetInstance().QueryTable(strSql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.QueryTable failed, iRet = %d.", iRet);
        return false;
    }

    if (iRowCount > 0) {
        return true;
    } else {
        return false;
    }
}

/* ------------------------------------------------------------
Function Name: CheckTrapInfoTable
Description  : 检查表中是否存在AgentIP字段，如果不存在AgentIP则创建此列
Others       :-------------------------------------------------------- */
mp_int32 AlarmDB::CheckTrapInfoTable()
{
    LOGGUARD("");
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    // check col if exists
    DbParamStream dps;
    mp_string checkTrapColumnExists =
        "select * from [sqlite_master] where name = 'TrapInfoTable' and sql like '%AgentIP%';";
    mp_int32 iRet = CDB::GetInstance().QueryTable(checkTrapColumnExists, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query col AgentIP from TrapInfoTable failed, iRet = %d.", iRet);
        return iRet;
    }

    // 如果没有AgentIP列，直接生成对应的列，并完成数据内容更新
    if (iRowCount == 0) {
        dps.Clear();
        mp_string addTrapColumn  = "alter table [TrapInfoTable] add AgentIP VARCHAR(30) NOT NULL DEFAULT ('Unknown');";
        iRet = CDB::GetInstance().ExecSql(addTrapColumn, dps);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Add column AgentIP in TrapInfoTable failed, iRet = %d.", iRet);
            return iRet;
        }
        COMMLOG(OS_LOG_INFO, "Add AgentIP column in table TrapInfoTable succeeded.");
    } else {
        COMMLOG(OS_LOG_INFO, "Table TrapInfoTable have AgentIP column, do nothing.");
    }
    return MP_SUCCESS;
}

mp_int32 CAlarmConfig::UpdateSnmpV3Param(const snmp_v3_param& stSnmpV3Param)
{
    LOGGUARD("");
    mp_int32 iRet = CConfigXmlParser::GetInstance().SetValue(
        CFG_SNMP_SECTION, CFG_CONTEXT_NAME, stSnmpV3Param.strContextName);
    iRet = CConfigXmlParser::GetInstance().SetValue(
        CFG_SNMP_SECTION, CFG_ENGINE_ID, stSnmpV3Param.strContextEngineId);
    iRet = CConfigXmlParser::GetInstance().SetValue(
        CFG_SNMP_SECTION, CFG_SECURITY_NAME, stSnmpV3Param.strSecurityName);
    mp_string strPrivatePw;
    EncryptStr(stSnmpV3Param.strPrivPassword, strPrivatePw);
    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_PRIVATE_PASSWOD, strPrivatePw);
    mp_string strAuthPw;
    EncryptStr(stSnmpV3Param.strAuthPassword, strAuthPw);
    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_AUTH_PASSWORD, strAuthPw);

    mp_string strTmp;
    if (stSnmpV3Param.iSecurityModel > 0) {
        std::stringstream ss;
        ss << stSnmpV3Param.iSecurityModel;
        ss >> strTmp;
        iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_SECURITY_MODEL, strTmp);
    }
    if (stSnmpV3Param.iSecurityLevel > 0) {
        std::stringstream ss;
        ss << stSnmpV3Param.iSecurityLevel;
        ss >> strTmp;
        iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_SECURITY_LEVEL, strTmp);
    }
    if (stSnmpV3Param.iAuthProtocol > 0) {
        std::stringstream ss;
        ss << stSnmpV3Param.iAuthProtocol;
        ss >> strTmp;
        iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_AUTH_PROTOCOL, strTmp);
    }
    if (stSnmpV3Param.iPrivProtocol > 0) {
        std::stringstream ss;
        ss << stSnmpV3Param.iPrivProtocol;
        ss >> strTmp;
        iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_PRIVATE_PROTOCOL, strTmp);
    }
    return iRet;
}

/* ------------------------------------------------------------
Function Name: GetSnmpV3Param
Description  : 从配置文件中获取snmp v3相关参数
Others       :-------------------------------------------------------- */
mp_void CAlarmConfig::GetSnmpV3Param(snmp_v3_param& stSnmpV3Param)
{
    LOGGUARD("");
    mp_string strPrivatePw;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, CFG_PRIVATE_PASSWOD, strPrivatePw);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get SNMP:private_password value from xml config failed");
        stSnmpV3Param.strPrivPassword = "";
    } else {
        // 解密
        DecryptStr(strPrivatePw, stSnmpV3Param.strPrivPassword);
    }

    mp_string strAuthPw;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, CFG_AUTH_PASSWORD, strAuthPw);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get SNMP:auth_password value from xml config failed");
        stSnmpV3Param.strAuthPassword = "";
    } else {
        // 解密
        DecryptStr(strAuthPw, stSnmpV3Param.strAuthPassword);
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(
        CFG_SNMP_SECTION, CFG_SECURITY_NAME, stSnmpV3Param.strSecurityName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get SNMP:security_name value from xml config failed");
        stSnmpV3Param.strSecurityName = "";
    }

    iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_SNMP_SECTION, CFG_SECURITY_MODEL, stSnmpV3Param.iSecurityModel);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get SNMP:security_model value from xml config failed");
        stSnmpV3Param.iSecurityModel = 0;
    }

    iRet =
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_SNMP_SECTION, CFG_AUTH_PROTOCOL, stSnmpV3Param.iAuthProtocol);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get SNMP:auth_protocol value from xml config failed");
        stSnmpV3Param.iAuthProtocol = 0;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_SNMP_SECTION, CFG_PRIVATE_PROTOCOL, stSnmpV3Param.iPrivProtocol);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get SNMP:private_protocol value from xml config failed");
        stSnmpV3Param.iPrivProtocol = 0;
    }
}
