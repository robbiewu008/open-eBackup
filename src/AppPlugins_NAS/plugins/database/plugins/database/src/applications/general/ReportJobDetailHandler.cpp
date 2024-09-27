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
#include "ReportJobDetailHandler.h"
#include <map>
#include "Path.h"
#include "log/Log.h"
#include "common/File.h"
#include "DBPluginPath.h"
#include "client/ClientInvoke.h"

using namespace GeneralDB;
using namespace AppProtect;

namespace {
    const mp_string MODULE = "ReportJobDetailHandler";
    const mp_string Log_Param_Sub_Job_ID = "SubJobID";
    const mp_string Log_Param_Agent_IP = "AgentNodeIP";
    const mp_string NULL_PARAM = "";
    const mp_string AGENT_NGINX_CONF_FILE_NAME = "nginx.conf";
    const mp_string BIND_IP_TAG = "listen";
    const mp_int32 UNDEFIND_ERRCODE = -1;
}

static std::map<LOG_LABEL_TYPE, LogData> LogDetailMap = {
    {LOG_LABEL_TYPE::EXEC_PREJOB_FAIL,
        {"plugin_execute_prerequisit_task_fail_label", {NULL_PARAM}, JobLogLevel::type::TASK_LOG_ERROR}},
    {LOG_LABEL_TYPE::EXEC_GEN_SUBJOB,
        {"plugin_excute_generate_subjob_label", {NULL_PARAM}, JobLogLevel::type::TASK_LOG_INFO}},
    {LOG_LABEL_TYPE::EXEC_GENJOB_FAIL,
        {"plugin_generate_subjob_fail_label", {NULL_PARAM}, JobLogLevel::type::TASK_LOG_ERROR}},
    {LOG_LABEL_TYPE::EXEC_GENJOB_SUCCESS,
        {"plugin_generate_subjob_success_label", {NULL_PARAM}, JobLogLevel::type::TASK_LOG_INFO}},
    {LOG_LABEL_TYPE::EXEC_BACKUP_SUBJOB_FAIL,
        {"plugin_backup_subjob_fail_label", {Log_Param_Sub_Job_ID}, JobLogLevel::type::TASK_LOG_ERROR}},
    {LOG_LABEL_TYPE::EXEC_RESTORE_SUBJOB_FAIL,
        {"plugin_restore_subjob_fail_label", {Log_Param_Sub_Job_ID}, JobLogLevel::type::TASK_LOG_ERROR}},
    {LOG_LABEL_TYPE::EXEC_LIVEMOUNT_SUBJOB_FAIL,
        {"plugin_live_mount_subjob_fail_label", {Log_Param_Sub_Job_ID}, JobLogLevel::type::TASK_LOG_ERROR}},
    {LOG_LABEL_TYPE::EXEC_CANCELLIVEMOUNT_SUBJOB_FAIL,
        {"plugin_cancel_livemount_subjob_fail_label", {Log_Param_Sub_Job_ID}, JobLogLevel::type::TASK_LOG_ERROR}},
    {LOG_LABEL_TYPE::EXEC_DELCOPY_SUBJOB_FAIL,
        {"plugin_delete_copy_subjob_fail_label", {Log_Param_Sub_Job_ID}, JobLogLevel::type::TASK_LOG_ERROR}},
    {LOG_LABEL_TYPE::START_EXEC_SUBJOB,
        {"agent_start_execute_sub_task_success_label",
            {Log_Param_Agent_IP, Log_Param_Sub_Job_ID}, JobLogLevel::type::TASK_LOG_INFO}}
};

std::shared_ptr<ReportJobDetailHandler> ReportJobDetailHandler::GetInstance()
{
    static std::shared_ptr<ReportJobDetailHandler> g_instance = std::make_shared<ReportJobDetailHandler>();
    return g_instance;
}

mp_int32 ReportJobDetailHandler::ReportJobDetailToFrame(const JobLogDetail &jobDetail)
{
    AppProtect::SubJobDetails subJobDetails;
    subJobDetails.__set_jobId(jobDetail.jobId);
    subJobDetails.__set_subJobId(jobDetail.subJobId);
    subJobDetails.__set_jobStatus(jobDetail.jobStatus);
    if (jobDetail.labelType != LOG_LABEL_TYPE::UNDEFIND_LABEL) {
        LogDetail logDetail {};
        std::vector<LogDetail> logDetails {};
        auto iter = LogDetailMap.find(jobDetail.labelType);
        if (iter != LogDetailMap.end()) {
            logDetail.__set_description(iter->second.label);
            logDetail.__set_level(iter->second.level);
            logDetail.__set_params(LogDetailParam(iter->second.params, jobDetail));
            if (jobDetail.errorCode != UNDEFIND_ERRCODE) {
                logDetail.__set_errorCode(jobDetail.errorCode);
            }
            logDetails.push_back(logDetail);
            subJobDetails.__set_logDetail(logDetails);
        }
    }
    AppProtect::ActionResult retValue;
    JobService::ReportJobDetails(retValue, subJobDetails);
    if (retValue.code != MP_SUCCESS) {
        HCP_Log(ERR, MODULE) << "Report job details to frame failed, jobId=" << jobDetail.jobId << ", subJobId=" <<
            jobDetail.subJobId << HCPENDLOG;
        return retValue.code;
    }
    HCP_Log(INFO, MODULE) << "Report job details to frame, jobId=" << jobDetail.jobId << ", subjobId=" <<
        jobDetail.subJobId << ", jobStatus=" << jobDetail.jobStatus << HCPENDLOG;
    return MP_SUCCESS;
}

std::vector<mp_string> ReportJobDetailHandler::LogDetailParam(const std::vector<mp_string>& parmas,
    const JobLogDetail &jobDetail)
{
    std::vector<mp_string> param;
    for (const auto& it : parmas) {
        if (it == Log_Param_Sub_Job_ID) {
            param.emplace_back(jobDetail.subJobId);
        } else if (it == Log_Param_Agent_IP) {
            param.emplace_back(GetAgentIp());
        } else if (it == NULL_PARAM) {
            continue;
        } else {
            HCP_Log(ERR, MODULE) << "No such params : " << it << HCPENDLOG;
        }
    }
    return param;
}

mp_string ReportJobDetailHandler::GetAgentIp()
{
    mp_string agentIp = "";
    mp_string strNginxConfFile = DBPluginPath::GetInstance()->GetNginxConfPath() + AGENT_NGINX_CONF_FILE_NAME;
    if (!Module::CFile::FileExist(strNginxConfFile.c_str())) {
        ERRLOG("Nginx config file does not exist, path is %s.", strNginxConfFile.c_str());
        return agentIp;
    }
    std::vector<mp_string> vecRlt;
    mp_int32 ret = Module::CIPCFile::ReadFile(strNginxConfFile, vecRlt);
    if (ret != MP_SUCCESS || vecRlt.size() == 0) {
        ERRLOG("Read nginx config file failed, ret = %d, size of vecRlt is %d.", ret, vecRlt.size());
        return agentIp;
    }
    for (mp_uint32 i = 0; i < vecRlt.size(); ++i) {
        mp_string strTmp = vecRlt[i];
        mp_string::size_type pos = strTmp.find(BIND_IP_TAG, 0);
        if (pos != mp_string::npos) {
            pos += strlen(BIND_IP_TAG.c_str());
            mp_string::size_type iColPos = strTmp.find_last_of(':');
            if (iColPos != mp_string::npos) {
                agentIp = strTmp.substr(pos, iColPos - pos);
                agentIp = Module::CMpString::Trim(agentIp);
            }
            break;
        }
    }
    return agentIp;
}