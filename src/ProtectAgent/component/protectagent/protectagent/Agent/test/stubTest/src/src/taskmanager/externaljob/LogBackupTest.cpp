#include "taskmanager/externaljob/LogBackupTest.h"
#include "taskmanager/externaljob/PluginLogBackup.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "common/JsonUtils.h"
#include "taskmanager/externaljob/JobStateDB.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "common/CMpTime.h"
#include "common/MpString.h"
#include <vector>
#include <set>
#include <dirent.h>
#include <unistd.h>
#include <chrono>

#include <iostream>

using namespace std;
using namespace AppProtect;

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubReadMetaDataTestAndvectorNoEmptySuccess(void *obj, mp_string &lastLogBackupPath, const mp_string &mainID, 
    std::vector<mp_string> &vecOutput)
{
    if (lastLogBackupPath == "/mnt/databackup/.agentlastlogbackup.meta") {
        vecOutput.push_back("123456");
        vecOutput.push_back("33333-44444_333-444");
    }
    if (lastLogBackupPath == "/mnt/databackup/.dmelastlogbackup.meta") {
        vecOutput.push_back("654321");
        vecOutput.push_back("11111-22222_111-222");
    }
    return MP_SUCCESS;
}

mp_void StubStrSplitTest(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep)
{
    if ((strText == "1641461699150-1641461700150_1111-2222") && (cSep == '-')) {
        vecTokens.push_back("1641461699150");
        vecTokens.push_back("1641461700150_1111");
        vecTokens.push_back("2222");
    }
    if (strText == "1641461699150-1641461700150_1111-2222" && cSep == '_') {
        vecTokens.push_back("1641461699150-1641461700150");
        vecTokens.push_back("1111-2222");
    }
    if (strText == "1111-2222" && cSep == '-') {
        vecTokens.push_back("1111");
        vecTokens.push_back("2222");
    }
    if (strText == "1641461699150-1641461700150" && cSep == '-') {
        vecTokens.push_back("1641461699150");
        vecTokens.push_back("1641461700150");
    }
}

mp_int32 StubAssembleLogDirNameTest(void *obj, const vector<mp_string> &vecAgentLastBackupMeta, 
    const vector<mp_string> &vecDmeLastBackupMeta, Json::Value &jsonExtendInfo, mp_string &logDirName)
{
    logDirName = "33333-66666_333-666";
    jsonExtendInfo["associatedCopies"].append("654321");
    return MP_SUCCESS;
}

std::shared_ptr<AppProtect::Job> StubGetRunJobByIdTest(void *obj, const mp_string& mainJobId)
{
    mp_string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e","appInfo" :{"type" : "HDFS",
    "subType" : "HDFSFileset",},"taskParams" :{"backupType" : "logBackup","copyFormat" : 0, "dataLayout":
    {"compression" : true,"deduption" : true,"encryption" : false,"extendInfo" : null},"filters" : [],"qos" : null,
    "scripts":{"failPostScript" : null,"postScript" : null,"preScript" : null}},"taskType" : 1,"repositories" :[
    {"path" : ["/mnt/databackup/test"],"repositoryType" : 3}]})";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    AppProtect::PluginJobData jobData = {"pluginName", "mainID", "subID", jsValue, 
        AppProtect::MainJobType::BACKUP_JOB, SubJobType::type::POST_SUB_JOB, 0};
    std::multimap<RepositoryDataType::type, std::vector<mp_string>> mountPoints;
    std::vector<mp_string> path;
    path.push_back("/mnt/databackup");
    jobData.mountPoints.insert(std::pair<RepositoryDataType::type, std::vector<mp_string>>(RepositoryDataType::type::LOG_REPOSITORY, path));
    std::shared_ptr<AppProtect::Job> pJob = AppProtect::PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    return pJob;
}

mp_int32 StubRetSuccess()
{
    return MP_SUCCESS;
}

mp_int32 StubRetFail()
{
    return MP_FAILED;
}

/*
* 用例名称：日志备份前置
* 前置条件：
* check点：1、前置正常，返回MP_SUCCESS
           2、获取挂载点失败，返回MP_FAILED，检查返回值
           3、读取元数据文件失败，返回MP_FAILED，检查返回值
           4、获取DME下发日志目录和数据副本ID失败，返回MP_FAILED，检查返回值
           5、创建临时日志目录失败，返回MP_FAILED，检查返回值
*/
TEST_F(LogBackupTest, LogBackup_Stub)
{
    mp_int32 ret;
    PluginLogBackup om;
    mp_string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e","appInfo" :{"type" : "HDFS",
    "subType" : "HDFSFileset",},"taskParams" :{"backupType" : "logBackup","copyFormat" : 0, "dataLayout":
    {"compression" : true,"deduption" : true,"encryption" : false,"extendInfo" : null},"filters" : [],"qos" : null,
    "scripts":{"failPostScript" : null,"postScript" : null,"preScript" : null}},"taskType" : 1,"repositories" :[
    {"path" : ["/mnt/databackup/test"],"type" : 3},{"path" : ["/mnt/databackup/test"],"type" : 2}]})";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    std::multimap<RepositoryDataType::type, std::vector<mp_string>> mountPoints;
    std::vector<mp_string> vecMountPoint = { "/mnt/databackup/test" };
    mountPoints.insert(make_pair(RepositoryDataType::type::LOG_REPOSITORY, vecMountPoint));

    PluginJobData data;
    data.param = jsValue;
    data.mountPoints = mountPoints;

    stub.set(ADDR(PluginLogBackup, ReadMetaData), StubReadMetaDataTestSuccess);
    stub.set(ADDR(PluginLogBackup, GetExtendInfo), StubGetExtendInfoTestSuccess);
    stub.set(ADDR(PluginLogBackup, WriteDMELastBackupMeta), StubWriteDMELastBackupMetaTestSuccess);
    stub.set(ADDR(PluginLogBackup, CreateLogDir), StubCreateLogDirTestSuccess);
    stub.set(ADDR(CMpFile, DirExist), StubDirNotExist);
    stub.set(ADDR(CMpFile, CreateDir), StubCreateDirSuccess);
    ret = om.LogBackup(data);
    EXPECT_EQ(ret, MP_SUCCESS);

    stub.set(ADDR(PluginLogBackup, ReadMetaData), StubReadMetaDataTestFail);
    ret = om.LogBackup(data);
    EXPECT_EQ(ret, MP_FAILED);

    stub.set(ADDR(PluginLogBackup, ReadMetaData), StubReadMetaDataTestSuccess);
    stub.set(ADDR(PluginLogBackup, GetExtendInfo), StubGetExtendInfoTestFail);
    ret = om.LogBackup(data);
    EXPECT_EQ(ret, MP_FAILED);

    stub.set(ADDR(PluginLogBackup, ReadMetaData), StubReadMetaDataTestSuccess);
    stub.set(ADDR(PluginLogBackup, GetExtendInfo), StubGetExtendInfoTestSuccess);
    stub.set(ADDR(PluginLogBackup, WriteDMELastBackupMeta), StubWriteDMELastBackupMetaTestFail);
    ret = om.LogBackup(data);
    EXPECT_EQ(ret, MP_FAILED);

    stub.set(ADDR(PluginLogBackup, ReadMetaData), StubReadMetaDataTestSuccess);
    stub.set(ADDR(PluginLogBackup, GetExtendInfo), StubGetExtendInfoTestSuccess);
    stub.set(ADDR(PluginLogBackup, WriteDMELastBackupMeta), StubWriteDMELastBackupMetaTestSuccess);
    stub.set(ADDR(PluginLogBackup, CreateLogDir), StubCreateLogDirTestFail);
    ret = om.LogBackup(data);
    EXPECT_EQ(ret, MP_FAILED);
}

/*
* 用例名称：创建临时日志目录
* 前置条件：1、无
* check点：1、创建成功，返回MP_SUCCESS
*/
TEST_F(LogBackupTest, StubCreateLogDir)
{
    mp_int32 ret;
    PluginLogBackup om;
    PluginJobData data;
    data.mainID = "1111";
    vector<mp_string> vec;
    mp_string latestDataCopyId = "33333";
    mp_string latestLogCopyName = "44444";

    stub.set(ADDR(CMpFile, DirExist), StubDirExist);
    stub.set(ADDR(CIPCFile, WriteFile), StubWriteFileTestSuccess);
    ret = om.CreateLogDir(vec, latestDataCopyId, latestLogCopyName, data);
    EXPECT_EQ(ret, MP_SUCCESS);
}

/*
* 用例名称：读取元数据文件
* 前置条件：无
* check点：1、元数据文件读取正常，返回MP_SUCCESS
           2、元数据文件不存在，创建失败，返回MP_FAILED
           3、数据文件读取失败，返回MP_FAILED
*/
TEST_F(LogBackupTest, ReadMetaDataFile)
{
    mp_int32 ret;
    PluginLogBackup om;
    mp_string lastLogBackupPath = "/test";
    mp_string mainID = "1111";
    vector<mp_string> vecOutput;

    stub.set(ADDR(CMpFile, FileExist), StubFileNotExist);
    stub.set(ADDR(CMpFile, CreateFile), StubCreateFileTestSuccess);
    stub.set(ADDR(CMpFile, ReadFile), StubReadFileTestSuccess);
    ret = om.ReadMetaData(lastLogBackupPath, mainID, vecOutput);
    EXPECT_EQ(ret, MP_SUCCESS);

    stub.set(ADDR(CMpFile, CreateFile), StubCreateFileTestFail);
    ret = om.ReadMetaData(lastLogBackupPath, mainID, vecOutput);
    EXPECT_EQ(ret, MP_FAILED);
    
    stub.set(ADDR(CMpFile, CreateFile), StubCreateFileTestSuccess);
    stub.set(ADDR(CMpFile, ReadFile), StubReadFileTestFail);
    ret = om.ReadMetaData(lastLogBackupPath, mainID, vecOutput);
    EXPECT_EQ(ret, MP_FAILED);
}

/*
* 用例名称：记录DME下发数据副本ID和日志目录元数据文件
* 前置条件：无
* check点：1、写入元数据正常，返回MP_SUCCESS
           2、创建失败，返回MP_FAILED
           3、写入失败，返回MP_FAILED
*/
TEST_F(LogBackupTest, WriteDMELastBackupMeta_Stub)
{
    mp_int32 ret;
    PluginLogBackup om;
    mp_string lastLogBackupPath = ""; 
    mp_string latestDataCopyId = "";
    mp_string latestLogCopyName = "";

    stub.set(ADDR(CMpFile, FileExist), StubFileNotExist);
    stub.set(ADDR(CMpFile, CreateFile), StubCreateFileTestSuccess);
    stub.set(ADDR(PluginLogBackup, WriteToFile), StubWriteFileTestSuccess);
    ret = om.WriteDMELastBackupMeta(lastLogBackupPath, latestDataCopyId, latestLogCopyName);
    EXPECT_EQ(ret, MP_SUCCESS);

    stub.set(ADDR(CMpFile, CreateFile), StubCreateFileTestFail);
    ret = om.WriteDMELastBackupMeta(lastLogBackupPath, latestDataCopyId, latestLogCopyName);
    EXPECT_EQ(ret, MP_FAILED);
    
    stub.set(ADDR(CMpFile, FileExist), StubFileNotExist);
    stub.set(ADDR(CMpFile, CreateFile), StubCreateFileTestSuccess);
    stub.set(ADDR(PluginLogBackup, WriteToFile), StubWriteFileTestFail);
    ret = om.WriteDMELastBackupMeta(lastLogBackupPath, latestDataCopyId, latestLogCopyName);
    EXPECT_EQ(ret, MP_FAILED);
}

/*
* 用例名称：查看备份类型是否日志备份
* 前置条件：无
* check点：1、如果是日志备份，返回MP_TRUE
           2、非备份任务或者非日志备份，返回MP_FALSE
*/
TEST_F(LogBackupTest, IsLogBackupJobType)
{
    mp_bool ret;
    PluginLogBackup om;
    mp_string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e","appInfo" :{"type" : "HDFS","subType" : "HDFSFileset",},
	"taskParams" :{"backupType" : "logBackup","copyFormat" : 0, "dataLayout":{"compression" : true,"deduption" : true,
    "encryption" : false,"extendInfo" : null},"filters" : [],"qos" : null,"scripts":{"failPostScript" : null,"postScript" : null,
    "preScript" : null}},"taskType" : 1})";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);

    PluginJobData data;
    data.param = jsValue;
    data.mainType = MainJobType::BACKUP_JOB;
    ret = om.IsLogBackupJob(data);
    EXPECT_EQ(ret, MP_TRUE);

    data.mainType = MainJobType::RESTORE_JOB;
    ret = om.IsLogBackupJob(data);
    EXPECT_EQ(ret, MP_FALSE);
}

/*
* 用例名称：获取DME下发的扩展信息内容中数据副本ID和日志目录名
* 前置条件：无
* check点：1、获取成功，检查副本ID和目录名是否一致
           2、日志仓数量为0，返回MP_FAILED
           3、扩展字段不含副本ID和目录名，获取失败，返回MP_FAILED
*/
TEST_F(LogBackupTest, GetDMEExtendInfo)
{
    mp_int32 ret;
    PluginLogBackup om;
    mp_string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e","appInfo" :{"type" : "HDFS",
    "subType" : "HDFSFileset",},"taskParams" :{"backupType" : "logBackup","copyFormat" : 0, "dataLayout":
    {"compression" : true,"deduption" : true,"encryption" : false,"extendInfo" : null},"filters" : [],"qos" : null,
    "scripts":{"failPostScript" : null,"postScript" : null,"preScript" : null}},"taskType" : 1,"repositories" :[
    {"path" : ["/mnt/databackup/test"],"type" : 3,"extendInfo": {"logBackup": {"latestDataCopyId": "dme_latest_data_copy_id",
    "latestLogCopyName": "dme_latest_log_dir_name"}}},{"path" : ["/mnt/databackup/test"],"type" : 2}]})";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);

    PluginJobData data;
    data.param = jsValue;
    mp_string latestDataCopyId;
    mp_string latestLogCopyName;
    ret = om.GetExtendInfo(data, latestDataCopyId, latestLogCopyName);
    EXPECT_EQ(ret, MP_SUCCESS);
    EXPECT_EQ(latestDataCopyId, "dme_latest_data_copy_id");
    EXPECT_EQ(latestLogCopyName, "dme_latest_log_dir_name");

    strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e","appInfo" :{"type" : "HDFS",
    "subType" : "HDFSFileset",},"taskParams" :{"backupType" : "logBackup","copyFormat" : 0, "dataLayout":
    {"compression" : true,"deduption" : true,"encryption" : false,"extendInfo" : null},"filters" : [],"qos" : null,
    "scripts":{"failPostScript" : null,"postScript" : null,"preScript" : null}},"taskType" : 1,"repositories" :[]})";
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    data.param = jsValue;
    ret = om.GetExtendInfo(data, latestDataCopyId, latestLogCopyName);
    EXPECT_EQ(ret, MP_FAILED);

    strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e","appInfo" :{"type" : "HDFS",
    "subType" : "HDFSFileset",},"taskParams" :{"backupType" : "logBackup","copyFormat" : 0, "dataLayout":
    {"compression" : true,"deduption" : true,"encryption" : false,"extendInfo" : null},"filters" : [],"qos" : null,
    "scripts":{"failPostScript" : null,"postScript" : null,"preScript" : null}},"taskType" : 1,"repositories" :[ 
    {"extendInfo":{"logBackup": {}}}]})";
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    data.param = jsValue;
    ret = om.GetExtendInfo(data, latestDataCopyId, latestLogCopyName);
    EXPECT_EQ(ret, MP_FAILED);

    strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e","appInfo" :{"type" : "HDFS",
    "subType" : "HDFSFileset",},"taskParams" :{"backupType" : "logBackup","copyFormat" : 0, "dataLayout":
    {"compression" : true,"deduption" : true,"encryption" : false,"extendInfo" : null},"filters" : [],"qos" : null,
    "scripts":{"failPostScript" : null,"postScript" : null,"preScript" : null}},"taskType" : 1,"repositories" :[ 
    {"extendInfo" : null}]})";
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    data.param = jsValue;
    ret = om.GetExtendInfo(data, latestDataCopyId, latestLogCopyName);
    EXPECT_EQ(ret, MP_FAILED);
}

/*
* 用例名称：组装上报DME的日志副本信息
* 前置条件：上次日志备份Agent成功，DME入库失败，且本次日志备份跨数据副本
* check点：1、组装成功，检查组装后日志目录名和数据副本列表个数
*/
TEST_F(LogBackupTest, AssembleReportDMECopyInfo)
{
    mp_int32 ret;
    PluginLogBackup om;
    PluginJobData data;
    Json::Value jobValue;
    {
        Json::Value extendInfo;
        {
            extendInfo["beginTime"] = "55555";
            extendInfo["endTime"] = "66666";
            extendInfo["beginSCN"] = "555";
            extendInfo["endSCN"] = "666";
        }
        jobValue["extendInfo"] = extendInfo;
    }
    stub.set(ADDR(AppProtectJobHandler, GetPostJobByMainId), StubGetRunJobByIdTest);
    stub.set(ADDR(PluginLogBackup, IsLogBackupJob), StubIsLogBackupJobTest);
    stub.set(ADDR(PluginLogBackup, ReadMetaData), StubReadMetaDataTestAndvectorNoEmptySuccess);
    stub.set(ADDR(PluginLogBackup, WriteToFile), StubWriteFileTestSuccess);

    ret = om.AssembleCopyInfo("111", jobValue);
    EXPECT_EQ(ret, MP_SUCCESS);
    EXPECT_EQ(jobValue["extendInfo"]["logDirName"].asString(), "111");
    EXPECT_EQ(jobValue["extendInfo"]["associatedCopies"].size(), 2);
}