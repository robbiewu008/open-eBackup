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
#include <vector>
#include "apps/appprotect/plugininterface/JobServiceHandlerTest.h"
#include "apps/appprotect/plugininterface/JobServiceHandler.h"
#include "message/curlclient/DmeRestClient.h"
#include "taskmanager/externaljob/PluginLogBackup.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "securecom/ConsistentHashRing.h"
#include "message/curlclient/RestClientCommon.h"
#include "common/Log.h"
#include "common/Types.h"
#include "xbsaclientcomm/File.h"
#include "stub.h"

typedef mp_void (*StubFuncType)(void);
typedef mp_void (CLogger::*LogType)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...);

static mp_int32 entryCount;

using namespace AppProtect;

static mp_void StubCLoggerLog(void){
    return;
}

static mp_void SleepStub(mp_void* pthis, mp_uint32 ms)
{
    return;
}

static mp_int32 StubFailed(mp_void* pthis){
    entryCount ++;
    return MP_FAILED;
}

static mp_int32 StubSuccess(mp_void* pthis){
    entryCount ++;
    return MP_SUCCESS;
}

static mp_int32 Stub_Succ(){
    return MP_SUCCESS;
}

static mp_bool StubFalse(mp_void* pthis){
    return MP_FALSE;
}

static mp_bool StubTrue(mp_void* pthis){
    return MP_TRUE;
}

mp_void JobServiceHandlerTestLogVoid(mp_void* pthis)
{
    return;
}

mp_int32 StubConvertStrToRspMsg(const std::string &rspstr, RestClientCommon::RspMsg &rspSt)
{
    entryCount ++;
    rspSt.errorCode != "0";
    rspSt.errorMessage = "test errorMessage";
    return MP_SUCCESS;
}

mp_bool StubAddNode(const std::string& node)
{
    if (entryCount++ == 0){
        return MP_TRUE;
    }
    return MP_FALSE;
}

mp_bool StubAssignNode(const std::string& data, std::string& node)
{
    if (entryCount++ == 0){
        return MP_TRUE;
    }
    return MP_FALSE;
}

mp_int32 StubGetUbcIpsByMainJobIdSucc(mp_void* pThis, const mp_string mainJobId, std::vector<mp_string>& ubcIps)
{
    return MP_SUCCESS;
}

TEST_F(JobServiceHandlerTest, AddNewJobTest) {
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    mp_stub.set(
        (mp_int32(AppProtect::AppProtectJobHandler::*)())ADDR(AppProtect::AppProtectJobHandler, GetUbcIpsByMainJobId),
        StubGetUbcIpsByMainJobIdSucc);
    std::vector<AppProtect::SubJob> jobs;
    AppProtect::ActionResult _return;
    jobservice::JobServiceHandler jobServices;
    jobServices.AddNewJob(_return, jobs);
    EXPECT_EQ(0, entryCount);

    AppProtect::SubJob subJob;
    subJob.jobId = "0001";
    subJob.jobName = "jobName";
    subJob.jobPriority = 0001;
    jobs.push_back(subJob);

    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubFailed);
    jobServices.AddNewJob(_return, jobs);
    EXPECT_EQ(1, entryCount);
}

TEST_F(JobServiceHandlerTest, ReportJobDetailsTest) {
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    AppProtect::ActionResult _return;
    AppProtect::SubJobDetails jobInfo;
    jobInfo.jobId = "0001";
    jobInfo.subJobId = "0001";
    jobInfo.jobStatus = AppProtect::SubJobStatus::FAILED;
    jobInfo.progress = 99;
    jobInfo.speed = 99;
    jobservice::JobServiceHandler jobServices;

    mp_stub.set(ADDR(AppProtectJobHandler, ReportJobDetails), StubFailed);
    jobServices.ReportJobDetails(_return, jobInfo);
    EXPECT_EQ(1, entryCount);
}

TEST_F(JobServiceHandlerTest, ReportCopyAdditionalInfoTest) {
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    AppProtect::ActionResult _return;
    std::string jobId;
    AppProtect::Copy copy;
    jobservice::JobServiceHandler jobServices;

    mp_stub.set(ADDR(AppProtect::PluginLogBackup, AssembleCopyInfo), StubSuccess);
    mp_stub.set(ADDR(jobservice::JobServiceHandler, RestoreRepositories), StubSuccess);
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubFailed);
    mp_stub.set(ADDR(CMpTime, DoSleep), SleepStub);
    jobServices.ReportCopyAdditionalInfo(_return, jobId, copy);
    EXPECT_NE(_return.code, 0);
}

TEST_F(JobServiceHandlerTest, ComputerFileLocationInMultiFileSystemTest) {
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    std::map<std::string, std::string> _return;
    std::vector<std::string> files;
    std::vector<std::string> fileSystems;
    jobservice::JobServiceHandler jobServices;
    fileSystems.push_back("strFs");
    fileSystems.push_back("strFs2");
    files.push_back("strFile");
    files.push_back("strFile2");

    mp_stub.set(&ConsistentHashRing::AddNode, StubAddNode);
    EXPECT_THROW(jobServices.ComputerFileLocationInMultiFileSystem(_return, files, fileSystems),AppProtectFrameworkException);
    
    entryCount = 0;
    mp_stub.set(ADDR(ConsistentHashRing, AddNode), StubTrue);
    mp_stub.set(ADDR(ConsistentHashRing, AssignNode), StubAssignNode);
    EXPECT_THROW(jobServices.ComputerFileLocationInMultiFileSystem(_return, files, fileSystems),AppProtectFrameworkException);
}

TEST_F(JobServiceHandlerTest, HandleRestResultTest) {
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    AppProtect::ActionResult _return;
    HttpResponse response;
    response.statusCode == 400;
    jobservice::JobServiceHandler jobServices;

    mp_stub.set(ADDR(RestClientCommon, ConvertStrToRspMsg), StubFailed);
    jobServices.HandleRestResult(_return, response);
    EXPECT_EQ(1, entryCount);
    mp_stub.set(ADDR(RestClientCommon, ConvertStrToRspMsg), StubConvertStrToRspMsg);
    jobServices.HandleRestResult(_return, response);
    EXPECT_EQ(2, entryCount);
}

TEST_F(JobServiceHandlerTest, RestoreRepositoriesTest) {
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    jobservice::JobServiceHandler jobServices;
    Json::Value copyValue;
    Json::Value copyValueSub;
    copyValueSub["id"] = "id";
    copyValueSub["type"] = "type";
    copyValueSub["role"] = "role";
    copyValueSub["protocol"] = "protocol";
    copyValueSub["isLocal"] = "isLocal";
    copyValueSub["remotePath"] = "remotePath";
    copyValueSub["remoteHost"] = "remoteHost";
    copyValueSub["extendInfo"]["fsId"] = "fsId";
    copyValueSub["type"] = "type";
    copyValue["repositories"].append(copyValueSub);

    jobServices.RestoreRepositories(copyValue);
    EXPECT_EQ(0, entryCount);
}


TEST_F(JobServiceHandlerTest, MountRepositoryByPluginTest) {
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    AppProtect::ActionResult _return;
    AppProtect::PrepareRepositoryByPlugin mountinfo;
    jobservice::JobServiceHandler jobServices;
    jobServices.MountRepositoryByPlugin(_return, mountinfo);
    EXPECT_EQ(0, entryCount);
}

TEST_F(JobServiceHandlerTest, UnMountRepositoryByPluginTest) {
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    AppProtect::ActionResult _return;
    AppProtect::PrepareRepositoryByPlugin mountinfo;
    jobservice::JobServiceHandler jobServices;
    jobServices.UnMountRepositoryByPlugin(_return, mountinfo);
    EXPECT_EQ(0, entryCount);
}

TEST_F(JobServiceHandlerTest, IsCurAgentFcOnSuccess)
{
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string extenInfo = "fibreChannel : {b00869a5-719f-404d-a2ce-fdb1cb3ca765:true}";
    jobservice::JobServiceHandler jobServices;
    jobServices.IsCurAgentFcOn(extenInfo);
    EXPECT_EQ(0, entryCount);
    mp_stub.set(&CJsonUtils::ConvertStringtoJson, Stub_Succ);
    bool ret = jobServices.IsCurAgentFcOn(extenInfo);
    EXPECT_EQ(ret, MP_FALSE);
}

TEST_F(JobServiceHandlerTest, IsCurAgentFcOnT)
{
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string extenInfo = "{\"fibreChannel\":\"dskljfdljfkdj\"}";
    jobservice::JobServiceHandler jobServices;
    jobServices.IsCurAgentFcOn(extenInfo);
    EXPECT_EQ(0, entryCount);
    extenInfo = "{\"fibreChannel\":\"{\"host\":\"dskljfdljfkdj\"}\"}";
    bool ret = jobServices.IsCurAgentFcOn(extenInfo);
    EXPECT_EQ(ret, MP_FALSE);
}

TEST_F(JobServiceHandlerTest, IsSanClientMountSuccess)
{
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string extenInfo = "fibreChannel : {b00869a5-719f-404d-a2ce-fdb1cb3ca765:true}";
    jobservice::JobServiceHandler jobServices;
    jobServices.IsSanClientMount(extenInfo);
    EXPECT_EQ(0, entryCount);
    mp_stub.set(&CJsonUtils::ConvertStringtoJson, Stub_Succ);
    bool ret = jobServices.IsSanClientMount(extenInfo);
    EXPECT_EQ(ret, MP_FALSE);
}

TEST_F(JobServiceHandlerTest, IsSanClientMountT)
{
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string extenInfo = "{\"sanclientInvolved\":\"true\"}";
    jobservice::JobServiceHandler jobServices;
    jobServices.IsSanClientMount(extenInfo);
    EXPECT_EQ(0, entryCount);
    bool ret = jobServices.IsSanClientMount(extenInfo);
    EXPECT_EQ(ret, MP_TRUE);
}

TEST_F(JobServiceHandlerTest, GetSanClientParamT)
{
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    Json::Value extendInfo;
    Json::Value sanClientParam;
    jobservice::JobServiceHandler jobServices;
    int ret = jobServices.GetSanClientParam(extendInfo, sanClientParam);
    EXPECT_EQ(ret, MP_FAILED);
    Json::Value agenta, agentb;
    agenta["id"] = "idstr1";
    agentb["id"] = "";
    extendInfo["agents"].append(agenta);
    extendInfo["agents"].append(agentb);
    ret = jobServices.GetSanClientParam(extendInfo, sanClientParam);
    EXPECT_EQ(ret, MP_FAILED);
    mp_stub.set(ADDR(CHost, GetHostSN), Stub_Succ);
    ret = jobServices.GetSanClientParam(extendInfo, sanClientParam);
    EXPECT_EQ(ret, MP_FAILED);
    agenta["sanClients"].append("str1dojfkdlj");
    agenta["sanClients"].append("str2jdkfjk");
    agentb["sanClients"].append("str1dojfkdljdff");
    agentb["sanClients"].append("str2jdkfjkddf");
    mp_stub.set(ADDR(CHost, GetHostSN), Stub_Succ);
    ret = jobServices.GetSanClientParam(extendInfo, sanClientParam);
    EXPECT_EQ(ret, MP_FAILED);
}

TEST_F(JobServiceHandlerTest, GetLunInfoT)
{
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    jobservice::JobServiceHandler jobServices;
    Json::Value sanClientParam ,sanClienta, sanClientb, luninfoa, luninfob;
    sanClienta["iqns"].append("iqnsstra");
    sanClientb["iqns"].append("iqnsstrb");
    luninfoa["lunName"] = "12";
    luninfoa["filesystemsize"] = 64;
    luninfoa["sanclientWwpn"] = "sanclientWwpn";
    luninfoa["lunId"] = "lunId";
    luninfoa["agentWwpn"] = "agentWwpna";
    luninfob["lunName"] = "13";
    luninfob["filesystemsize"] = 64;
    luninfob["sanclientWwpn"] = "sanclientWwpnb";
    luninfob["lunId"] = "lunIdb";
    luninfob["agentWwpn"] = "agentWwpnb";
    sanClienta["luninfo"].append(luninfoa);
    sanClienta["luninfo"].append(luninfob);
    sanClientParam["sanClients"].append(sanClienta);
    mp_string mountProtocol = "mountProtocol";
    mp_string repositoryType = "repositoryType";
    mp_string remotePath = "remotePath";
    mp_string ret = jobServices.GetLunInfo(sanClientParam, mountProtocol, repositoryType, remotePath);
    EXPECT_EQ(ret, "repositoryType:");
}

TEST_F(JobServiceHandlerTest, CheckAndCreateDataturboLinkSuccess)
{
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    AppProtect::PrepareRepositoryByPlugin mountinfo;
    mp_stub.set(ADDR(RestClientCommon, ConvertStrToRspMsg), StubSuccess);
    jobservice::JobServiceHandler jobServices;
    mp_string errMsg;
    mp_int32 errCode;
    jobServices.MountNasFilesystem(mountinfo, errMsg, errCode);
    mp_stub.reset(ADDR(RestClientCommon, ConvertStrToRspMsg));
    EXPECT_EQ(0, entryCount);
    mountinfo.extendInfo = "{\"info\":\"info\"}";
    int ret = jobServices.MountFileIoSystem(mountinfo);
    EXPECT_EQ(ret, MP_FAILED);
    mountinfo.extendInfo = "faslkdjkjf";
    ret = jobServices.MountFileIoSystem(mountinfo);
    EXPECT_EQ(ret, MP_FAILED);
    AppProtect::ActionResult _return;
    AppProtect::AlarmDetails alarm;
    jobServices.SendAlarm(_return, alarm);
    jobServices.ClearAlarm(_return, alarm);
    EXPECT_EQ(0, entryCount);
}

TEST_F(JobServiceHandlerTest, MountDataturboFileSystemSuccess)
{
    Stub mp_stub;
    entryCount = 0;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    AppProtect::PrepareRepositoryByPlugin mountinfo;
    mp_stub.set(ADDR(RestClientCommon, ConvertStrToRspMsg), StubSuccess);
    jobservice::JobServiceHandler jobServices;
    jobServices.MountDataturboFilesystem(mountinfo);
    mp_stub.reset(ADDR(RestClientCommon, ConvertStrToRspMsg));
    EXPECT_EQ(0, entryCount);
}

/**
 * @brief 添加IP白名单成功
 *
 */
TEST_F(JobServiceHandlerTest, AddIpWhiteListSuccess)
{
    Stub mp_stub;
    mp_stub.set(&CLogger::Log, StubCLoggerLog);
    AppProtect::ActionResult _return;
    jobservice::JobServiceHandler jobServices;
    std::string jobId = "1-1-1-1";
    std::string ipListStr = "172.1.1.1";
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSuccess);
    jobServices.AddIpWhiteList(_return, jobId, ipListStr);
}