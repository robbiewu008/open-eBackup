/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepOracleNative.h
 * @brief  Contains function declarations oracle native backup base class
 * @version 1.0.0
 * @date 2020-01-27
 * @author wangguitao 00510599
 */
#ifndef AGENT_BACKUP_STEP_ORACLENATIVE
#define AGENT_BACKUP_STEP_ORACLENATIVE

#include <map>
#include <vector>
#include <sstream>
#include "jsoncpp/include/json/json.h"
#include "jsoncpp/include/json/value.h"
#include "common/Types.h"
#include "taskmanager/TaskStep.h"
#include "device/BackupVolume.h"

typedef enum { DATA_VOLUMES_BIT = 1, LOG_VOLUMES_BIT = 2, PRODUCT_VOLUMES_BIT = 4 } VolumeCheckFlag;

static const mp_string DISK_PATH = "/dev/";
static const mp_string VG_NAME_PRE_DATA = "vgDataBk";
static const mp_string LV_NAME_PRE_DATA = "lvDataBk";
static const mp_string FS_MOUNTPATH_PRE_DATA = "/tmp/advbackup/data/";
static const mp_string VG_NAME_PRE_LOG = "vgLogBk";
static const mp_string LV_NAME_PRE_LOG = "lvLogBk";
static const mp_string FS_MOUNTPATH_PRE_LOG = "/tmp/advbackup/log/";
static const mp_string DG_NAME_PRE_DATA = "dgDataBk";
static const mp_string DG_NAME_PRE_LOG = "dgLogBk";

static const mp_string WIN_ORACLE_ORACLENATIVEBACKUP_DATA = "oraclenativebackup.bat";
static const mp_string WIN_ORACLE_ORACLENATIVEBACKUP_LOG = "oraclenativearchiveback.bat";
static const mp_string WIN_ORACLE_ORACLENATIVE_BACKUPSTATUS = "oraclebackupstatus.bat";
static const mp_string WIN_ORACLE_STORAGEINFO = "oraclestorinfo.bat";
static const mp_string WIN_ORACLE_NATIVE_CHECKDB_STATUS = "oraclecheckdbstatus.bat";
static const mp_string WIN_ORACLE_NATIVE_RESTORE = "oraclenativerestore.bat";
static const mp_string WIN_ORACLE_NATIVE_LIVEMOUNT = "oraclenativelivemount.bat";
static const mp_string WIN_ORACLE_NATIVE_CANCEL_LIVEMOUNT = "oraclenativeclivemount.bat";
static const mp_string WIN_ORACLE_NATIVE_INSTRESTORE = "oraclenativeinstrestore.bat";
static const mp_string WIN_ORACLE_NATIVE_EXPIRE_COPY = "oraclenativeexpire.bat";
static const mp_string WIN_ORACLE_NATIVE_DISMOUNT_MEDIUM = "oraclenativedismount.bat";
static const mp_string WIN_ORACLE_NATIVE_STORE_DETAIL = "oraclestoredetail.bat";
static const mp_string WIN_ORACLE_NATIVE_CHECKAUTH = "oraclecheckauth.bat";
static const mp_string WIN_ORACLE_NATIVE_MOVEDBF = "oraclenativemovedbf.bat";
static const mp_string WIN_ORACLE_NATIVE_CLEAR_LEFTOVER_RES = "clearleftoverfsres.bat";

class TaskStepOracleNative : public TaskStep {
public:
    TaskStepOracleNative(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    TaskStepOracleNative();
    virtual ~TaskStepOracleNative();
    virtual mp_int32 Init(const Json::Value& param);
    virtual mp_int32 Run();
    virtual mp_int32 Cancel();
    virtual mp_int32 Cancel(Json::Value& respParam);
    virtual mp_int32 Stop(const Json::Value& param);
    virtual mp_int32 Update(const Json::Value& param);
    virtual mp_int32 Update(Json::Value& param, Json::Value& respParam);
    virtual mp_int32 Finish(const Json::Value& param);
    virtual mp_int32 Finish(Json::Value& param, Json::Value& respParam);

    mp_int32 Redo(mp_string& innerPID);

protected:
    mp_string dbName;
    mp_string dbUUID;
    mp_string instName;
    mp_string dbUser;
    mp_string dbPwd;
    mp_string asmInstance;
    mp_string asmUser;
    mp_string asmPwd;
    mp_string oracleHome;
    mp_int32 dbType;

    mp_int32 hostRole;
    mp_int32 taskType;
    mp_int32 storType;

    std::vector<BackupVolume> dataVols;
    std::vector<BackupVolume> logVols;
    std::map<mp_string, mp_int32> m_mapRmanTaskStatus;

    mp_string m_strpfileparam;
    mp_string m_strpfileuuid;

protected:
    mp_int32 InitialDBInfo(const Json::Value& param);
    mp_int32 InitialVolInfo(const Json::Value& param, mp_char flag);
    mp_int32 InitExtrasParams(const Json::Value& param);
    mp_int32 InitStorType(const Json::Value& param);
    mp_int32 CheckVolsValid(mp_char flag);
    mp_int32 GetDiskListByWWN(const std::vector<BackupVolume>& vols, mp_string& diskList);
    mp_int32 JsonArray2VolumeArr(const std::vector<Json::Value>& jsonArr, std::vector<BackupVolume>& volArr,
                                 const mp_string& metaFlag, const mp_string& volumeType);
    mp_void BuildQueryRmanTaskStatusParam(mp_string& strParam, const mp_string& taskMode);
    mp_int32 AnalyseQueryRmanStatusScriptRst(std::vector<mp_string>& vecResult, mp_int32& status, mp_int32& progress);
    mp_int32 FillSpeedAndProgress(const mp_string& vecResult, mp_int32& status, mp_int32& progress);
    mp_int32 ConvertStatus(const mp_string& status);
    mp_int32 ConvertBackupSpeed(const mp_string& speed);
    mp_void BuildStopRmanTaskScriptParam(mp_string& strParam, mp_int32 TaskType);
    mp_int32 BuildPfileInfo();
};

#endif
