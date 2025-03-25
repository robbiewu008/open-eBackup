/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file AppProtectService.h
 * @brief Implement for application protect
 * @version 1.1.0
 * @date 2021-10-30
 * @author wangguitao 00510599
 */

#ifndef APPLICATION_PROTECT_SERVICE_H
#define APPLICATION_PROTECT_SERVICE_H

#include <mutex>
#include <regex>
#include "common/Types.h"
#include "message/rest/message_process.h"
#include "apps/appprotect/plugininterface/ProtectService.h"
#include "servicecenter/thriftservice/include/IThriftClient.h"
#include "taskmanager/externaljob/Job.h"


class AppProtectService {
public:
    static std::shared_ptr<AppProtectService> GetInstance();

    mp_int32 Init();
    mp_void SetSanclientFailedPreJob(const mp_string &taskId);
    mp_int32 SanclientJobForUbc(Json::Value& jvReq, CRequestMsg& req);
    mp_int32 WakeUpJob(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 AbortJob(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 DeliverJobStatus(const mp_string &strAppType, CRequestMsg& req, CResponseMsg& rsp);
    mp_void CreateCheckArchiveThread();
    mp_int32 SanclientMount(const Json::Value& jvReq, const mp_string& taskId,
                            std::vector<AppProtect::FilesystemInfo> &filesysteminfo, const mp_string &lanFreeSwitch);
    mp_int32 EnvCheck(const mp_string& jobid);
    mp_int32 CreateLun(const std::vector<mp_string> &agentwwpns,
                       const std::vector<mp_string> &wwpns,
                       std::vector<AppProtect::FilesystemInfo> &filesysteminfo, const mp_string &taskid);
    mp_int32 CreateLunISCSI(const mp_string &agentIqn, const mp_string &iqn,
                            std::vector<AppProtect::FilesystemInfo> &filesysteminfo,
                            const mp_string &taskid);
    mp_int32 CleanEnv(const mp_string &taskid);
    mp_int32 DeleteLunInfo(const Json::Value &jvReq);

    ~AppProtectService();
private:
    AppProtectService();
    std::smatch GetTaskIdFromUrl(const std::string &url);
    mp_int32 CreateLunForWwpns(const std::vector<mp_string> &wwpns,
                               std::vector<AppProtect::FilesystemInfo> &filesysteminfo,
                               const mp_string &taskid, AppProtect::LunInfo &luninfo);
    mp_int32 CreateLunForWwpnsV2(const mp_string &backupJobID, const AppProtect::FilesystemInfo &fsInfo,
                                 const mp_string &taskid, AppProtect::LunInfo &luninfo, mp_bool &flag);
    mp_int32 CreateLunForWwpnsV3(const AppProtect::FilesystemInfo &fsInfo,
                                 const mp_string &taskid, AppProtect::LunInfo &luninfo, mp_bool &flag);
    mp_int32 CreateLunForWwpnsV4(const AppProtect::FilesystemInfo &fsInfo,
                                 const mp_string &taskid, AppProtect::LunInfo &luninfo, mp_bool &flag);
    mp_int32 CreateLunForIqns(std::vector<AppProtect::FilesystemInfo> &filesysteminfo,
                              const mp_string &taskid, AppProtect::LunInfo &luninfo,
                              const mp_string &strIP, mp_string &unidirectionalAuthPwd);
    mp_int32 CreateLunForIqnsV2(const mp_string &backupJobID, const AppProtect::FilesystemInfo &fsInfo,
                                mp_string &unidirectionalAuthPwd, AppProtect::LunInfo &luninfo, const mp_string &strIP);
    mp_int32 CreateLunForIqnsV3(const AppProtect::FilesystemInfo &fsInfo, mp_string &unidirectionalAuthPwd,
                                AppProtect::LunInfo &luninfo, const mp_string &strIP);
    void SanclientMountEX(Json::Value &repositories, Json::Value &remotePath,
                          AppProtect::MountNasParam &param,
                          AppProtect::FilesystemInfo &fileinfo,
                          StorageRepository &stRep);
    mp_int32 MountType(const mp_string& lanFreeSwitch,  AppProtect::MountNasParam &param,
                       AppProtect::FilesystemInfo &fileinfo, std::vector<AppProtect::FilesystemInfo> &filesysteminfo);
    std::shared_ptr<AppProtect::ProtectServiceIf> GetProtectServiceClient(
        std::shared_ptr<thriftservice::IThriftClient>& pThriftClient);
    void ReportLunInfo(const mp_string &taskid, Json::Value &jvReq);
    void AssignmentLunId(mp_int32 &lunid);
    void ReportLunInfoforCluster(const mp_string &taskid, Json::Value &jvReq);
    mp_int32 GetChapPass(mp_string &unidirectionalAuthPwd);
    mp_void StructToJson(const AppProtect::LunInfo &lunInfo, Json::Value &luninfojson);
    void LunInfoForAgent(const AppProtect::FilesystemInfo &fileinfo,
        const mp_string &fileioName, AppProtect::LunInfo &luninfo,
        const mp_int32 &lunid, const mp_string &filesystemsize);
    void HandleDatasize(mp_string &datasize, const Json::Value &jvReq);
    mp_void GetCopiesID(const Json::Value &jvReq, std::vector<mp_string> &backupCopiesID, mp_string &logCopyID);
    mp_int32 CopyLogMeta(const mp_string &logRepositoryPath, const mp_string &cacheRepositoryPath,
        const std::vector<mp_string> &backupCopiesID);
    mp_void GetLogRepAndCacheRep(const AppProtect::FilesystemInfo &fileinfo, mp_bool &isLogRepositoryMounted,
        mp_string &logRepositoryPath, mp_string &cacheRepositoryPath);
    mp_void InsertLunInfo(const AppProtect::LunInfo &lunInfo, Json::Value &lunInfoJson);
    mp_int32 SanclientMountParaCheck(const Json::Value &jvReq);
    mp_int32 StringToLonglong(mp_int64 &result, const std::string &str);
    mp_int32 InitFilesystemInfo(AppProtect::FilesystemInfo &fileInfo, const mp_string &dataSize,
        const mp_int32 &taskType, const mp_string &logCopyID);

private:
    static std::shared_ptr<AppProtectService> g_instance;
    static std::mutex m_mutex;
    static std::mutex m_mutex_write;
    static std::mutex m_mutex_lunidlist;
    static std::mutex m_mutex_luninfolist;
    static std::mutex m_mutex_errorcodelist;
    static std::mutex m_mutex_mountpoint;
    static std::mutex m_mutex_sanclientprejobfailedset;
    std::map<mp_string, std::vector<mp_string>> m_mountPoint;
    std::map<mp_string, Json::Value> m_LuninfoList;
    std::map<mp_string, mp_int32> m_ErrorcodeList;
    std::set<mp_string> m_sanclientPreJobFailedSet;
    std::vector<AppProtect::LunInfo> m_lunInfos;
    std::list<int> m_lunidList;
    mp_bool m_jvReqGenerated = false;
};

#endif
