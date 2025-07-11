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
#include "taskmanager/externaljob/JobStateDB.h"
#include "common/DB.h"
#include "common/Log.h"
namespace AppProtect {
JobStateDB& JobStateDB::GetInstance()
{
    static JobStateDB instance;
    return instance;
}

mp_int32 JobStateDB::QueryAllJob(std::vector<PluginJobData>& result)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff << "select " << g_PluginJobs_AppType << ", " << g_PluginJobs_MainID << ", " << g_PluginJobs_SubID
        << ", " << g_PluginJobs_MainType << ", " << g_PluginJobs_SubType << ", " << g_PluginJobs_Status
        << ", " << g_PluginJobs_DmeIPS << ", " << g_PluginJobs_GenerateTime << ", " << g_PluginJobs_RunEnable
        << " from " << g_PluginJobs << ";";
    mp_string sql = buff.str();

    DbParamStream dps;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
    mp_int32 iRet = CDB::GetInstance().QueryTable(sql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        ERRLOG("db.QueryTable failed, iRet = %d.", iRet);
        return iRet;
    }

    mp_string strTmp;
    result.reserve(iRowCount);
    for (mp_int32 iRow = 0; iRow < iRowCount; ++iRow) {
        PluginJobData jobData;
        readBuff >> jobData.appType;
        readBuff >> jobData.mainID;
        readBuff >> jobData.subID;
        readBuff >> strTmp;
        jobData.mainType = MainJobType(atoint32(strTmp.c_str()));
        readBuff >> strTmp;
        jobData.subType = SubJobType::type(atoint32(strTmp.c_str()));
        readBuff >> strTmp;
        jobData.status = atoint32(strTmp.c_str());
        readBuff >> strTmp;
        CMpString::StrSplit(jobData.dmeIps, strTmp, STR_COMMA.front());
        readBuff >> strTmp;
        jobData.timeBorn = atoint64(strTmp.c_str());
        readBuff >> strTmp;
        jobData.canRunStatus = atoint32(strTmp.c_str());
        result.emplace_back(jobData);
    }
    INFOLOG("QueryAllJob successfully.");
    return iRet;
}

mp_int32 JobStateDB::QueryJob(const mp_string& mainId, const mp_string& subId, PluginJobData& jobData)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff << "select " << g_PluginJobs_AppType << ", " << g_PluginJobs_MainID << ", " << g_PluginJobs_SubID
        << ", " << g_PluginJobs_MainType << ", " << g_PluginJobs_SubType << ", " << g_PluginJobs_Status
        << ", " << g_PluginJobs_DmeIPS << ", " << g_PluginJobs_GenerateTime << ", " << g_PluginJobs_RunEnable
        << " from " << g_PluginJobs << " where "
        << g_PluginJobs_MainID << " == ? and " << g_PluginJobs_SubID << " == ?;";
    mp_string sql = buff.str();

    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
    DbParamStream dps;
    dps << DbParam(mainId) << DbParam(subId);
    mp_int32 iRet = CDB::GetInstance().QueryTable(sql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        ERRLOG("db.QueryTable failed, jobId=%s, subJobId=%s, iRet=%d.", mainId.c_str(), subId.c_str(), iRet);
        return iRet;
    }
    if (iRowCount == 0) {
        return MP_NOEXISTS;
    }

    mp_string strTmp;
    readBuff >> jobData.appType;
    readBuff >> jobData.mainID;
    readBuff >> jobData.subID;
    readBuff >> strTmp;
    jobData.mainType = MainJobType(atoint32(strTmp.c_str()));
    readBuff >> strTmp;
    jobData.subType = SubJobType::type(atoint32(strTmp.c_str()));
    readBuff >> strTmp;
    jobData.status = atoint32(strTmp.c_str());
    readBuff >> strTmp;
    CMpString::StrSplit(jobData.dmeIps, strTmp, STR_COMMA.front());
    readBuff >> strTmp;
    jobData.timeBorn = atoint64(strTmp.c_str());
    readBuff >> strTmp;
    jobData.canRunStatus = atoint32(strTmp.c_str());
    return iRet;
}

mp_int32 JobStateDB::InsertRecord(const PluginJobData& jobData)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff << "insert into " << g_PluginJobs << "(" << g_PluginJobs_AppType << ", " << g_PluginJobs_MainID << ", "
        << g_PluginJobs_SubID << ", " << g_PluginJobs_MainType << ", " << g_PluginJobs_SubType << ", "
        << g_PluginJobs_Status << ", " << g_PluginJobs_DmeIPS << ", " << g_PluginJobs_GenerateTime
        << ", " << g_PluginJobs_RunEnable << ") values(?, ?, ?, ?, ?, ?, ?, ?, ?);";
    mp_string sql = buff.str();

    DbParamStream dps;
    dps << DbParam(jobData.appType) << DbParam(jobData.mainID) << DbParam(jobData.subID)
        << DbParam(mp_int32(jobData.mainType)) << DbParam(mp_int32(jobData.subType)) << DbParam(jobData.status)
        << DbParam(CMpString::StrJoin(jobData.dmeIps, STR_COMMA))
        << DbParam(jobData.timeBorn) << DbParam(jobData.canRunStatus);

    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        ERRLOG("InsertRecord failed, iRet=%d, jobId=%s, subJobId=%s.",
            iRet, jobData.mainID.c_str(), jobData.subID.c_str());
    } else {
        INFOLOG("InsertRecord successfully, jobId=%s, subJobId=%s.", jobData.mainID.c_str(), jobData.subID.c_str());
    }
    return iRet;
}

mp_int32 JobStateDB::DeleteRecord(const mp_string& mainId, const mp_string& subId)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff << "delete from " << g_PluginJobs
        << " where " << g_PluginJobs_MainID << " == ? and " << g_PluginJobs_SubID << " == ?;";
    mp_string sql = buff.str();

    DbParamStream dps;
    dps << DbParam(mainId) << DbParam(subId);

    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        ERRLOG("DeleteRecord failed, iRet=%d, jobId=%s, subJobId=%s.", iRet, mainId.c_str(), subId.c_str());
    } else {
        INFOLOG("DeleteRecord successfully, jobId=%s, subJobId=%s.", mainId.c_str(), subId.c_str());
    }
    return iRet;
}

mp_int32 JobStateDB::DeleteRecord(const mp_string& mainId)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff << "delete from " << g_PluginJobs << " where " << g_PluginJobs_MainID << " == ?;";
    mp_string sql = buff.str();

    DbParamStream dps;
    dps << DbParam(mainId);

    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        ERRLOG("DeleteRecord failed, iRet=%d, jobId=%s.", iRet, mainId.c_str());
    } else {
        INFOLOG("DeleteRecord successfully, jobId=%s.", mainId.c_str());
    }
    return iRet;
}

mp_int32 JobStateDB::UpdateStatus(const mp_string& mainId, const mp_string& subId, mp_uint32 status)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff << "update " << g_PluginJobs << " set " << g_PluginJobs_Status << " = ? where "
        << g_PluginJobs_MainID << " == ? and " << g_PluginJobs_SubID << " == ?;";
    mp_string sql = buff.str();

    DbParamStream dps;
    dps << DbParam(status) << DbParam(mainId) << DbParam(subId);

    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        ERRLOG("UpdateStatus failed, iRet=%d, jobId=%s, subJobId=%s.", iRet, mainId.c_str(), subId.c_str());
    } else {
        INFOLOG("UpdateStatus successfully, jobId=%s, subJobId=%s.", mainId.c_str(), subId.c_str());
    }
    return iRet;
}

mp_int32 JobStateDB::UpdateMountPoints(const mp_string& mainId, const mp_string& subId,
    const std::vector<mp_string>& mountPoints)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff << "update " << g_PluginJobs << " set " << g_PluginJobs_MountPoints << " = ? where "
        << g_PluginJobs_MainID << " == ? and " << g_PluginJobs_SubID << " == ?;";
    mp_string sql = buff.str();

    DbParamStream dps;
    dps << DbParam(CMpString::StrJoin(mountPoints, STR_COMMA)) << DbParam(mainId) << DbParam(subId);

    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        ERRLOG("UpdateMountPoints failed, iRet=%d, jobId=%s, subJobId=%s.", iRet, mainId.c_str(), subId.c_str());
    } else {
        INFOLOG("UpdateMountPoints successfully, jobId=%s, subJobId=%s.", mainId.c_str(), subId.c_str());
    }
    return iRet;
}

mp_int32 JobStateDB::UpdateRunEnable(const mp_string& mainId, mp_int32 runEnable)
{
    LOGGUARD("");
    std::ostringstream buff;
    buff << "update " << g_PluginJobs << " set " << g_PluginJobs_RunEnable << " = ? where "
        << g_PluginJobs_MainID << " == ?;";
    mp_string sql = buff.str();

    DbParamStream dps;
    dps << DbParam(runEnable) << DbParam(mainId);

    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        ERRLOG("UpdateRunEnable failed, iRet=%d, jobId=%s.", iRet, mainId.c_str());
    } else {
        INFOLOG("UpdateRunEnable successfully, jobId=%s.", mainId.c_str());
    }
    return iRet;
}
}
