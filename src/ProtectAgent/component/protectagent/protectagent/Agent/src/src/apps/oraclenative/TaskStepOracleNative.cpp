#include "apps/oraclenative/TaskStepOracleNative.h"

#include <map>
#include <vector>
#include <sstream>

#include "common/ErrorCode.h"
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "securecom/UniqueId.h"
#include "apps/oracle/OracleDefines.h"
#include "array/array.h"
#include "array/disk.h"
#include "device/BackupVolume.h"
#include "host/host.h"
#include "common/Utils.h"


using namespace std;

TaskStepOracleNative::TaskStepOracleNative(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStep(id, taskId, name, ratio, order),
    m_strpfileparam(""),
    m_strpfileuuid("")
{}

TaskStepOracleNative::~TaskStepOracleNative()
{
    ClearString(dbPwd);
    ClearString(asmPwd);
}

mp_int32 TaskStepOracleNative::Init(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::Run()
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::Cancel()
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::Cancel(Json::Value& respParam)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::Stop(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::Update(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::Update(Json::Value& param, Json::Value& respParam)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::Finish(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::Finish(Json::Value& param, Json::Value& respParam)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::Redo(mp_string& innerPID)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::InitialDBInfo(const Json::Value& param)
{
    static const mp_string keyDbInfo = "dbInfo";
    static const mp_string keyDbName = "dbName";
    static const mp_string keyDbUUID = "dbUUID";
    static const mp_string keyInstName = "instName";
    static const mp_string keyDbUser = "dbUser";
    static const mp_string keyDbPwd = "dbPwd";
    static const mp_string keyAsmInstance = "ASMInstance";
    static const mp_string keyAsmUser = "ASMUser";
    static const mp_string keyAsmPwd = "ASMPwd";
    static const mp_string keyDbType = "dbType";

    if (!param.isMember(keyDbInfo)) {
        COMMLOG(OS_LOG_ERROR, "param have no dbinfo key %s.", keyDbInfo.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

    GET_JSON_STRING(param[keyDbInfo], keyDbName, dbName);
    CHECK_FAIL_EX(CheckParamStringEnd(dbName, 0, ORACLE_PLUGIN_MAX_DBNAME));
    CHECK_FAIL_EX(CheckParamValid(dbName));
    GET_JSON_STRING(param[keyDbInfo], keyDbUUID, dbUUID);
    CHECK_FAIL_EX(CheckParamStringEnd(dbUUID, 0, ORACLE_PLUGIN_MAX_UUID));
    GET_JSON_STRING(param[keyDbInfo], keyInstName, instName);
    CHECK_FAIL_EX(CheckParamStringEnd(instName, 0, ORACLE_PLUGIN_MAX_DBINSTANCENAME));
    CHECK_FAIL_EX(CheckParamValid(instName));
    GET_JSON_STRING(param[keyDbInfo], keyDbUser, dbUser);
    CHECK_FAIL_EX(CheckParamStringEnd(dbUser, 0, ORACLE_PLUGIN_MAX_NSERNAME));
    CHECK_FAIL_EX(CheckParamValid(dbUser));
    GET_JSON_STRING(param[keyDbInfo], keyDbPwd, dbPwd);
    CHECK_FAIL_EX(CheckParamStringEnd(dbPwd, 0, ORACLE_PLUGIN_MAX_STRING));
    CHECK_FAIL_EX(CheckParamValid(dbPwd));
    GET_JSON_STRING_OPTION(param[keyDbInfo], keyAsmInstance, asmInstance);
    CHECK_FAIL_EX(CheckParamStringEnd(asmInstance, 0, ORACLE_PLUGIN_MAX_DBINSTANCENAME));
    CHECK_FAIL_EX(CheckParamValid(asmInstance));
    GET_JSON_STRING_OPTION(param[keyDbInfo], keyAsmUser, asmUser);
    CHECK_FAIL_EX(CheckParamStringEnd(asmUser, 0, ORACLE_PLUGIN_MAX_NSERNAME));
    CHECK_FAIL_EX(CheckParamValid(asmUser));
    GET_JSON_STRING_OPTION(param[keyDbInfo], keyAsmPwd, asmPwd);
    CHECK_FAIL_EX(CheckParamStringEnd(asmPwd, 0, ORACLE_PLUGIN_MAX_STRING));
    CHECK_FAIL_EX(CheckParamValid(asmPwd));
    GET_JSON_INT32(param[keyDbInfo], keyDbType, dbType);
    CHECK_FAIL_EX(CheckParamInteger32(dbType, 0, ORACLE_PLUGIN_MAX_DBTYPE));
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::InitExtrasParams(const Json::Value& param)
{
    if (!param.isMember(keyHostRole)) {
        COMMLOG(OS_LOG_ERROR, "param have no hostRole key %s.", keyHostRole.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    GET_JSON_INT32(param, keyHostRole, hostRole);
    CHECK_FAIL_EX(CheckParamInteger32(hostRole, 0, ORACLE_PLUGIN_MAX_HOSTROLE));
    COMMLOG(OS_LOG_DEBUG, "get hostRole is %d.", hostRole);

    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::InitStorType(const Json::Value& param)
{
    if (!param.isMember(keyStorage)) {
        COMMLOG(OS_LOG_ERROR, "param have no keyStorage key %s.", keyStorage.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

    Json::Value mediaInfo = param[keyStorage];
    if (!mediaInfo.isMember(keyStorType)) {
        COMMLOG(OS_LOG_WARN, "dpp message have no key storage \'storType\'.");
        storType = ORA_STORTYPE_FC;
    } else {
        GET_JSON_INT32(mediaInfo, keyStorType, storType);
        CheckParamInteger32(storType, ORA_STORTYPE_NAS, ORA_STORTYPE_FC);
    }
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::InitialVolInfo(const Json::Value& param, mp_char flag)
{
    static const mp_string keyDataVolumes = "dataVolumes";
    static const mp_string keyLogVolumes = "logVolumes";

    // data volume
    if (flag == DATA_VOLUMES_BIT) {
        std::vector<Json::Value> jsonDataVols;
        mp_int32 iRet = CJsonUtils::GetJsonArrayJson(param, keyDataVolumes, jsonDataVols);
        if (iRet != MP_SUCCESS) {
            return iRet;
        }

        iRet = JsonArray2VolumeArr(jsonDataVols, dataVols, "1", "1");
        if (iRet != MP_SUCCESS) {
            return iRet;
        }
    }

    // log volume
    if (flag == LOG_VOLUMES_BIT) {
        std::vector<Json::Value> jsonLogVols;
        mp_int32 iRet = CJsonUtils::GetJsonArrayJson(param, keyLogVolumes, jsonLogVols);
        if (iRet != MP_SUCCESS) {
            return iRet;
        }
        iRet = JsonArray2VolumeArr(jsonLogVols, logVols, "0", "0");
        if (iRet != MP_SUCCESS) {
            return iRet;
        }
    }

    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::JsonArray2VolumeArr(const std::vector<Json::Value>& jsonArr,
    std::vector<BackupVolume>& volArr, const mp_string& metaFlag, const mp_string& volumeType)
{
    static const mp_string keyIsExtend = "isExtend";
    static const mp_string keyIsMetaData = "isMetaData";

    for (mp_size i = 0; i < jsonArr.size(); i++) {
        const Json::Value& vol = jsonArr[i];

        if (!vol.isObject()) {
            COMMLOG(OS_LOG_ERROR, "JsonArray2VolumeArr: The value is not Json object.");
            volArr.clear();
            return ERROR_COMMON_INVALID_PARAM;
        } else {
            if (!vol.isMember("mediumID")) {
                COMMLOG(OS_LOG_ERROR, "JsonArray2VolumeArr: The value have no mediumID.");
                volArr.clear();
                return ERROR_COMMON_INVALID_PARAM;
            }

            mp_string isExtend = "0";
            if (vol.isMember(keyIsExtend)) {
                isExtend = vol[keyIsExtend].asString();
            }

            mp_string isMetaData = metaFlag;
            if (vol.isMember(keyIsMetaData)) {
                isMetaData = vol[keyIsMetaData].asString();
            }
            BackupVolume bkVol(vol["mediumID"].asString(), isExtend, isMetaData, volumeType);
            volArr. emplace_back(bkVol);
        }
    }

    return MP_SUCCESS;
}

/* check vols valid
 0000 0000
       pld
p: product volumes
l: log volumes
d: data volumes
*/
mp_int32 TaskStepOracleNative::CheckVolsValid(mp_char flag)
{
    // need to check data volumes valid
    if ((flag & DATA_VOLUMES_BIT) == DATA_VOLUMES_BIT) {
        if (dataVols.empty()) {
            COMMLOG(OS_LOG_ERROR, "param have no data volumes data.");
            return ERROR_COMMON_INVALID_PARAM;
        }
    }

    if ((flag & LOG_VOLUMES_BIT) == LOG_VOLUMES_BIT) {
        if (logVols.empty()) {
            COMMLOG(OS_LOG_ERROR, "param have no log volumes data.");
            return ERROR_COMMON_INVALID_PARAM;
        }
    }
    return MP_SUCCESS;
}

// get relationship of wwn and disk by wwn list that is send by interface
mp_int32 TaskStepOracleNative::GetDiskListByWWN(const vector<BackupVolume>& vols, mp_string& diskList)
{
    LOGGUARD("");
    // get all disk list
    CHost host;
    vector<host_lun_info_t> lunList;
    mp_int32 iRet = host.GetDiskInfo(lunList);
    if (iRet != MP_SUCCESS || lunList.empty()) {
        COMMLOG(OS_LOG_ERROR, "get disk list failed, iRet is %d", iRet);
        return ERROR_DISK_GET_DISK_INFO_FAILED;
    }

    diskList = "";
    ostringstream oss;
    for (vector<BackupVolume>::const_iterator iter = vols.begin(); iter != vols.end(); ++iter) {
        mp_string volWwn = iter->GetVolWwn();
        vector<host_lun_info_t>::iterator lunIter;
        for (lunIter = lunList.begin(); lunIter != lunList.end(); ++lunIter) {
            if (iter->GetVolWwn() == lunIter->wwn) {
                COMMLOG(OS_LOG_DEBUG, "The disk(%s) have find.", volWwn.c_str());
#ifdef WIN32
                oss << lunIter->diskNumber << STR_DASH << iter->GetIsExtend() << STR_DASH << iter->GetIsMetaData()
                    << STR_COMMA;
#else
                oss << lunIter->deviceName << STR_DASH << iter->GetIsExtend() << STR_DASH << iter->GetIsMetaData();
                (iter->GetVolumeType() == "0") ? (oss << STR_DASH << "log" << STR_COMMA) :
                                                 (oss << STR_DASH << "data" << STR_COMMA);
#endif
                break;
            }
        }
        if (lunIter == lunList.end()) {
            COMMLOG(OS_LOG_ERROR, "The disk(%s) isn't exist.", volWwn.c_str());
            return ERROR_DISK_GET_DISK_INFO_FAILED;
        }
    }

    diskList = oss.str();
    diskList = diskList.substr(0, diskList.length() - 1);
    COMMLOG(OS_LOG_DEBUG, "Get diskList is %s.", diskList.c_str());
    return MP_SUCCESS;
}

mp_void TaskStepOracleNative::BuildQueryRmanTaskStatusParam(mp_string& strParam, const mp_string& taskMode)
{
    strParam = ORACLE_SCRIPTPARAM_INSTNAME + instName + NODE_COLON +
        ORACLE_SCRIPTPARAM_DBNAME + dbName + NODE_COLON +
        ORACLE_SCRIPTPARAM_DBUSERNAME + dbUser + NODE_COLON +
        ORACLE_SCRIPTPARAM_DBPASSWORD + dbPwd + NODE_COLON +
        ORACLE_SCRIPTPARAM_ASMINSTANCE + instName +
        NODE_COLON + ORACLE_SCRIPTPARAM_ASMUSERNAME + asmUser + NODE_COLON +
        ORACLE_SCRIPTPARAM_ASMPASSWOD + asmPwd + NODE_COLON +
        ORACLE_SCRIPTPARAM_ORACLE_HOME + oracleHome + NODE_COLON +
        ORACLE_SCIPRTPARAM_QUERY_TASKMODE + taskMode;
        ClearString(dbPwd);
        ClearString(asmPwd);
}

/*
result format:[status;sofar;totalwork;progress]---COMPLETED;136320;136320;100
rman备份的主要状态
RUNNING,RUNNING WITH WARNINGS,RUNNING WITH ERRORS,COMPLETED,COMPLETED WITH WARNINGS,COMPLETED WITH ERRORS,FAILED
 */
mp_int32 TaskStepOracleNative::AnalyseQueryRmanStatusScriptRst(
    std::vector<mp_string>& vecResult, mp_int32& speed, mp_int32& progress)
{
    for (std::vector<mp_string>::iterator iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
        mp_int32 iRet = FillSpeedAndProgress(*iter, speed, progress);
        if (iRet == MP_FAILED) {
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::FillSpeedAndProgress(const mp_string& vecResult, mp_int32& speed, mp_int32& progress)
{
    // find 1st(;) speed
    mp_size idxSep = vecResult.find(STR_SEMICOLON);
    if (idxSep == mp_string::npos) {
        COMMLOG(OS_LOG_ERROR, "Get backup speed failed when find 1nd separator, speed info [%s].", vecResult.c_str());
        return MP_FAILED;
    }
    mp_string strSpeed = vecResult.substr(0, idxSep);
    if (!strSpeed.empty()) {
        speed = ConvertBackupSpeed(strSpeed);
    }

    static const std::size_t idx1 = 1;
    // find 2nd(;) sofar
    mp_size idxSepSec = vecResult.find(STR_SEMICOLON, idxSep + idx1);
    if (idxSepSec == mp_string::npos) {
        COMMLOG(OS_LOG_ERROR,
            "Get backup progress sofar failed when find 2nd separator, speed info [%s].",
            vecResult.c_str());
        return MP_FAILED;
    }
    mp_int64 iSofar = atoi(vecResult.substr(idxSep + idx1, (idxSepSec - idxSep) - idx1).c_str());
    if (idxSepSec == mp_string::npos) {
        COMMLOG(OS_LOG_ERROR,
            "Get backup progress totalwork failed when find 3nd separator, speed info [%s].",
            vecResult.c_str());
        return MP_FAILED;
    }
    mp_size idxSepTrd = vecResult.find(STR_SEMICOLON, idxSepSec + idx1);
    mp_int64 iTotalWork = atoi(vecResult.substr(idxSepSec + idx1, (idxSepTrd - idxSepSec) - idx1).c_str());
    if (iTotalWork == 0) {
        progress = 0;
    } else {
        mp_int32 iCardinality = 100;
        progress = mp_int32(((mp_double)iSofar / (mp_double)iTotalWork) * iCardinality);
    }

    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNative::ConvertStatus(const mp_string& status)
{
    // RUNNING,RUNNING WITH WARNINGS,RUNNING WITH ERRORS,COMPLETED,COMPLETED WITH WARNINGS,COMPLETED WITH ERRORS,FAILED
    mp_int32 iStatus = 0;

    std::map<mp_string, mp_int32>::iterator iter = m_mapRmanTaskStatus.find(status);
    if (iter != m_mapRmanTaskStatus.end()) {
        iStatus = iter->second;
    } else {
        iStatus = STATUS_ABORTED;
        COMMLOG(OS_LOG_ERROR, "Can not convert rman status %s, set default value.", status.c_str());
    }

    return iStatus;
}

mp_int32 TaskStepOracleNative::ConvertBackupSpeed(const mp_string& speed)
{
    mp_string strSpeed = speed;
    mp_int32 iSpeed = 0;
    std::size_t idxKB = strSpeed.find("K");
    std::size_t idxMB = strSpeed.find("M");
    std::size_t idxGB = strSpeed.find("G");
    strSpeed.erase(strSpeed.end() - 1);
    static const mp_int32 iCardinality = 1024;
    if (idxKB != mp_string::npos) {
        iSpeed = atof(strSpeed.c_str());
    } else if (idxMB != mp_string::npos) {
        iSpeed = atof(strSpeed.c_str()) * iCardinality;
    } else if (idxGB != mp_string::npos) {
        iSpeed = atof(strSpeed.c_str()) * iCardinality * iCardinality;
    } else {
        iSpeed = 0;
    }
    if (iSpeed < 0) {
        COMMLOG(OS_LOG_ERROR, "speed(%s), transform faild.", speed.c_str());
        iSpeed = 0;
    }
    
    return iSpeed;
}

mp_void TaskStepOracleNative::BuildStopRmanTaskScriptParam(mp_string& strParam, mp_int32 TaskType)
{
    std::ostringstream oss;
    oss << ORACLE_SCRIPTPARAM_INSTNAME << instName << NODE_COLON
        << ORACLE_SCRIPTPARAM_DBNAME << dbName << NODE_COLON
        << ORACLE_SCRIPTPARAM_DBUSERNAME << dbUser << NODE_COLON
        << ORACLE_SCRIPTPARAM_DBPASSWORD << dbPwd << NODE_COLON
        << ORACLE_SCRIPTPARAM_RMANTASKTYPE << TaskType << NODE_COLON
        << ORACLE_SCRIPTPARAM_TASKINNERID << m_taskStepInnerPID;
    ClearString(dbPwd);
    strParam = oss.str();
}

mp_int32 TaskStepOracleNative::BuildPfileInfo()
{
    Json::Reader jsonReader;
    Json::Value jsonValue;
    if (!jsonReader.parse(m_strpfileparam, jsonValue)) {
        COMMLOG(OS_LOG_ERROR, "pfileparam JsonData is invalid");
        return MP_FAILED;
    }
    if (jsonValue.empty()) {
        COMMLOG(OS_LOG_ERROR, "pfileparam JsonData is empty");
        return MP_FAILED;
    }

    m_strpfileuuid = CUniqueID::GetInstance().GetString();
    mp_string pfilename = "pfile" + m_strpfileuuid;
    mp_int32 iRet = CIPCFile::WriteJsonStrToKVFile(pfilename, jsonValue, STR_EQUAL);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "taskid %s, build restore pfile param fail ret %d.", m_taskId.c_str(), iRet);
        m_strpfileuuid.clear();
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "taskid %s, build restore pfile param sucss ", m_taskId.c_str());
    return MP_SUCCESS;
}