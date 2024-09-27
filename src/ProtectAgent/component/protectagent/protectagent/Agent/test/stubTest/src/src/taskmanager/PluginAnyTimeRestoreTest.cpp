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
#include "taskmanager/PluginAnyTimeRestoreTest.h"
#include "taskmanager/externaljob/PluginAnyTimeRestore.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"
#include "common/Log.h"
#include "securecom/RootCaller.h"
#include <iostream>
using namespace AppProtect;

namespace {
mp_void LogTest()
{}
#define DoLogTest()                                                                                                    \
    do {                                                                                                               \
        stub.set(ADDR(CLogger, Log), LogTest);                                                                         \
    } while (0)

mp_int32 mockopenfile(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.push_back("testdir1;1~2323;1~22323");
    vecOutput.push_back("testdir2;2323~4343;22323~23636");
    vecOutput.push_back("testdir3;2322~4345;22322~23640");
    return MP_SUCCESS;
}

mp_int32 mockGetTimeStampLogDirList(void *obj, const mp_time& timestamp, std::vector<mp_string>& logdirlist)
{
    logdirlist.push_back("testdir1");
    logdirlist.push_back("testdir2;");
    return MP_SUCCESS;
}

mp_int32 mockParseRestoreMetaInfo(void *obj, mp_string& metapath)
{
    return MP_SUCCESS;
}

mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    if (iCommandID == (mp_int32)ROOT_COMMAND_CAT) {
        if (strParam == "metapath") {
            pvecResult->push_back("dirname");
            pvecResult->push_back("dirname;1;1");
            pvecResult->push_back("dirname;1~9;1~9");
        }
    }
    return MP_SUCCESS;
}

mp_int32 StubSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_int32 StubFailed(mp_void* pthis)
{
    return MP_FAILED;
}

mp_int32 ComposeNewRepositorySuccess(const std::vector<mp_string>& mountPoint, StorageRepository& stRep,
    Json::Value& jsonRep_new)
{
    return MP_SUCCESS;
}

mp_int32 ComposeNewRepositoryFailed(const std::vector<mp_string>& mountPoint, StorageRepository& stRep,
    Json::Value& jsonRep_new)
{
    return MP_FAILED;
}
}  // namespace
/*
 * 用例名称：计算任意时间恢复日志副本函数成功
 * 前置条件：组合copy函数返回成功
 * check点：ComputerAnyTimeRestoreLogPath 函数在ComposeNewCopy函数正常返回时且传入参数
 *          没问题ComputerAnyTimeRestoreLogPath返回成功
 */
TEST_F(PluginAnyTimeRestoreTest, ComputerAnyTimeRestoreLogPathTimeSuccess)
{
    DoLogTest();
    std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e",
    "requestId" : "742222ad-721e-4243-a410-1eab3691f9e3",
    "subTaskType" : null,
    "taskParams" : {"restoreType" : "normalRestore"},
    "copies" :[{
        "protectSubObject" :[{"name" : "/liuxiaoxiang/happyFolder"}]
        }],
    "taskType" : 2,
    "extendInfo" : {
        "restoreCopyId": "sdaweaweawe",
        "restoreTimestamp": "1231455"
    }
    })";
    stub.set(&PluginAnyTimeRestore::ComposeNewRepository, ComposeNewRepositorySuccess);
    Json::Value jobData;
    CJsonUtils::ConvertStringtoJson(strValue, jobData);
    auto anyTimeRestore = PluginAnyTimeRestore();
    std::vector<mp_string> mountPoint;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    mp_int32 iRet = anyTimeRestore.ComputerAnyTimeRestoreLogPath(jobData, mountPoint, stRep, jsonRep_new);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 用例名称：计算任意时间恢复日志副本函数成功
 * 前置条件：组合copy函数返回成功
 * check点：ComputerAnyTimeRestoreLogPath 函数在ComposeNewCopy函数正常返回时且传入参数
 *          没问题ComputerAnyTimeRestoreLogPath返回成功
 */
TEST_F(PluginAnyTimeRestoreTest, ComputerAnyTimeRestoreLogPathInfoFailed)
{
    DoLogTest();
    std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e",
    "requestId" : "742222ad-721e-4243-a410-1eab3691f9e3",
    "subTaskType" : null,
    "taskParams" : {"restoreType" : "normalRestore"},
    "copies" :[{
        "protectSubObject" :[{"name" : "/liuxiaoxiang/happyFolder"}]
        }],
    "taskType" :2
    })";
    stub.set(&PluginAnyTimeRestore::ComposeNewRepository, ComposeNewRepositoryFailed);
    Json::Value jobData;
    CJsonUtils::ConvertStringtoJson(strValue, jobData);
    auto anyTimeRestore = PluginAnyTimeRestore();
    std::vector<mp_string> mountPoint;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    mp_int32 iRet = anyTimeRestore.ComputerAnyTimeRestoreLogPath(jobData, mountPoint, stRep, jsonRep_new);
    EXPECT_EQ(MP_FAILED, iRet);
}

/*
 * 用例名称：计算任意时间恢复日志副本函数成功
 * 前置条件：组合copy函数返回成功
 * check点：ComputerAnyTimeRestoreLogPath 函数在ComposeNewCopy函数正常返回时且传入参数
 *          没问题ComputerAnyTimeRestoreLogPath返回成功
 */
TEST_F(PluginAnyTimeRestoreTest, ComputerAnyTimeRestoreLogPathSCNSuccess)
{
    DoLogTest();
    std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e",
    "requestId" : "742222ad-721e-4243-a410-1eab3691f9e3",
    "subTaskType" : null,
    "taskParams" : {"restoreType" : "normalRestore"},
    "copies" :[{
        "protectSubObject" :[{"name" : "/liuxiaoxiang/happyFolder"}],
    }],
    "taskType" : 2,
    "extendInfo" : {
        "restoreCopyId": "sdaweaweawe",
        "restoreScn": "1231455"
    }
    })";
    stub.set(&PluginAnyTimeRestore::ComposeNewRepository, ComposeNewRepositorySuccess);
    Json::Value jobData;
    CJsonUtils::ConvertStringtoJson(strValue, jobData);
    auto anyTimeRestore = PluginAnyTimeRestore();
    std::vector<mp_string> mountPoint;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    mp_int32 iRet = anyTimeRestore.ComputerAnyTimeRestoreLogPath(jobData, mountPoint, stRep, jsonRep_new);
    EXPECT_EQ(MP_SUCCESS, iRet);
}
/*
 * 用例名称：计算任意时间恢复日志副本函数成功
 * 前置条件：组合copy函数返回成功
 * check点：ComputerAnyTimeRestoreLogPath 函数在ComposeNewCopy函数正常返回时且传入参数
 *          没问题ComputerAnyTimeRestoreLogPath返回成功
 */
TEST_F(PluginAnyTimeRestoreTest, ComputerAnyTimeRestoreLogPathRestestorinfoFailed)
{
    DoLogTest();
    std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e",
    "requestId" : "742222ad-721e-4243-a410-1eab3691f9e3",
    "subTaskType" : null,
    "taskParams" : {"restoreType" : "normalRestore"},
    "copies" :[{
        "protectSubObject" :[{"name" : "/liuxiaoxiang/happyFolder"}]
        }],
    "taskType" : 2,
    "extendInfo" : {
        "restoreCopyId": "sdaweaweawe"
    }
    })";
    stub.set(&PluginAnyTimeRestore::ComposeNewRepository, ComposeNewRepositoryFailed);
    Json::Value jobData;
    CJsonUtils::ConvertStringtoJson(strValue, jobData);
    auto anyTimeRestore = PluginAnyTimeRestore();
    std::vector<mp_string> mountPoint;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    mp_int32 iRet = anyTimeRestore.ComputerAnyTimeRestoreLogPath(jobData, mountPoint, stRep, jsonRep_new);
    EXPECT_EQ(MP_FAILED, iRet);
}
/*
 * 用例名称：计算任意时间恢复日志副本函数成功
 * 前置条件：组合copy函数返回成功
 * check点：ComputerAnyTimeRestoreLogPath 函数在ComposeNewCopy函数正常返回时且传入参数
 *          没问题ComputerAnyTimeRestoreLogPath返回成功
 */
TEST_F(PluginAnyTimeRestoreTest, ComputerAnyTimeRestoreLogPathCopyinfofailed)
{
    DoLogTest();
    std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e",
    "requestId" : "742222ad-721e-4243-a410-1eab3691f9e3",
    "subTaskType" : null,
    "taskParams" : {"restoreType" : "normalRestore"},
    "taskType" : 2,
    "extendInfo" : {
        "restoreCopyId": "sdaweaweawe"
    }
    })";
    stub.set(&PluginAnyTimeRestore::ComposeNewRepository, ComposeNewRepositorySuccess);
    Json::Value jobData;
    CJsonUtils::ConvertStringtoJson(strValue, jobData);
    auto anyTimeRestore = PluginAnyTimeRestore();
    std::vector<mp_string> mountPoint;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    mp_int32 iRet = anyTimeRestore.ComputerAnyTimeRestoreLogPath(jobData, mountPoint, stRep, jsonRep_new);
    EXPECT_EQ(MP_FAILED, iRet);
}
/*
 * 用例名称：计算任意时间恢复日志副本函数成功
 * 前置条件：组合copy函数返回成功
 * check点：ComputerAnyTimeRestoreLogPath 函数在ComposeNewCopy函数正常返回时且传入参数
 *          没问题ComputerAnyTimeRestoreLogPath返回成功
 */
TEST_F(PluginAnyTimeRestoreTest, ComputerAnyTimeRestoreLogPathFailed)
{
    DoLogTest();
    std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e",
    "requestId" : "742222ad-721e-4243-a410-1eab3691f9e3",
    "subTaskType" : null,
    "taskParams" : {"restoreType" : "normalRestore"},
    "copies" :[{
        "protectSubObject" :[{"name" : "/liuxiaoxiang/happyFolder"}],
    }],
    "taskType" : 2,
    "extendInfo" : {
        "restoreCopyId": "sdaweaweawe",
        "restoreScn": "1231455"
    }
    })";
    stub.set(&PluginAnyTimeRestore::ComposeNewRepository, ComposeNewRepositoryFailed);
    Json::Value jobData;
    CJsonUtils::ConvertStringtoJson(strValue, jobData);
    auto anyTimeRestore = PluginAnyTimeRestore();
    std::vector<mp_string> mountPoint;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    mp_int32 iRet = anyTimeRestore.ComputerAnyTimeRestoreLogPath(jobData, mountPoint, stRep, jsonRep_new);
    EXPECT_EQ(MP_FAILED, iRet);
}

/*
 * 用例名称：计算任意时间恢复日志副本函数成功
 * 前置条件：组合copy函数返回成功
 * check点：ComputerAnyTimeRestoreLogPath 函数在ComposeNewCopy函数正常返回时且传入参数
 *          没问题ComputerAnyTimeRestoreLogPath返回成功
 */
TEST_F(PluginAnyTimeRestoreTest, ParseRestoreMetaInfoSuccess)
{
    DoLogTest();
    std::string path = "xxx";
    stub.set(&CMpFile::ReadFile, mockopenfile);
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    auto anyTimeRestore = PluginAnyTimeRestore();
    mp_int32 iRet = anyTimeRestore.ParseRestoreMetaInfo(path);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 用例名称：组合copy
 * 前置条件：
 * check点：1、Mock log 2、主任务初始化成功
 */
TEST_F(PluginAnyTimeRestoreTest, ParseRestoreMetaInfoFailed)
{
    DoLogTest();
    std::string path = "";
    stub.set(&CMpFile::ReadFile, mockopenfile);
    auto anyTimeRestore = PluginAnyTimeRestore();
    mp_int32 iRet = anyTimeRestore.ParseRestoreMetaInfo(path);
    EXPECT_EQ(MP_FAILED, iRet);
}

/*
 * 用例名称：组合copy
 * 前置条件：
 * check点：1、Mock log 2、主任务初始化成功
 */
TEST_F(PluginAnyTimeRestoreTest, GetTimeStampLogDirListSuccess)
{
    DoLogTest();
    std::string path = "xxx";
    stub.set(&CMpFile::ReadFile, mockopenfile);
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    auto anyTimeRestore = PluginAnyTimeRestore();
    std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e",
    "requestId" : "742222ad-721e-4243-a410-1eab3691f9e3",
    "subTaskType" : null,
    "taskParams" : {"restoreType" : "normalRestore"},
    "copies" :[{
        "protectSubObject" :[{"name" : "/liuxiaoxiang/happyFolder"}],
    }],
    "taskType" : 2,
    "extendInfo" : {
        "restoreCopyId": "sdaweaweawe",
        "restoreTimestamp": "1234"
    }
    })";
    stub.set(&PluginAnyTimeRestore::ComposeNewRepository, ComposeNewRepositorySuccess);
    Json::Value jobData;
    CJsonUtils::ConvertStringtoJson(strValue, jobData);
    std::vector<mp_string> mountPoint;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    mp_int32 iRet = anyTimeRestore.ComputerAnyTimeRestoreLogPath(jobData, mountPoint, stRep, jsonRep_new);
    EXPECT_EQ(MP_SUCCESS, iRet);
    iRet = anyTimeRestore.ParseRestoreMetaInfo(path);
    EXPECT_EQ(MP_SUCCESS, iRet);
    std::vector<std::string> dirVec;
    iRet = anyTimeRestore.GetTimeStampLogDirList(dirVec);
    EXPECT_EQ(MP_FAILED, iRet);
}

/*
 * 用例名称：组合copy
 * 前置条件：
 * check点：1、Mock log 2、主任务初始化成功
 */
TEST_F(PluginAnyTimeRestoreTest, GetTimeStampLogDirListFailed)
{
    DoLogTest();
    std::string path = "xxx";
    stub.set(&CMpFile::ReadFile, mockopenfile);
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    auto anyTimeRestore = PluginAnyTimeRestore();
    std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e",
    "requestId" : "742222ad-721e-4243-a410-1eab3691f9e3",
    "subTaskType" : null,
    "taskParams" : {"restoreType" : "normalRestore"},
    "copies" :[{
        "protectSubObject" :[{"name" : "/liuxiaoxiang/happyFolder"}],
    }],
    "taskType" : 2,
    "extendInfo" : {
        "restoreCopyId": "sdaweaweawe",
        "restoreTimestamp": "9999"
    }
    })";
    stub.set(&PluginAnyTimeRestore::ComposeNewRepository, ComposeNewRepositorySuccess);
    Json::Value jobData;
    CJsonUtils::ConvertStringtoJson(strValue, jobData);
    std::vector<mp_string> mountPoint;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    mp_int32 iRet = anyTimeRestore.ComputerAnyTimeRestoreLogPath(jobData, mountPoint, stRep, jsonRep_new);
    EXPECT_EQ(MP_SUCCESS, iRet);
    iRet = anyTimeRestore.ParseRestoreMetaInfo(path);
    EXPECT_EQ(MP_SUCCESS, iRet);
    std::vector<std::string> dirVec;
    iRet = anyTimeRestore.GetTimeStampLogDirList(dirVec);
    EXPECT_EQ(MP_FAILED, iRet);
}

/*
 * 用例名称：组合copy
 * 前置条件：
 * check点：1、Mock log 2、主任务初始化成功
 */
TEST_F(PluginAnyTimeRestoreTest, GetSCNLogDirListSuccess)
{
    DoLogTest();
    std::string path = "xxx";
    stub.set(&CMpFile::ReadFile, mockopenfile);
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    auto anyTimeRestore = PluginAnyTimeRestore();
    std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e",
    "requestId" : "742222ad-721e-4243-a410-1eab3691f9e3",
    "subTaskType" : null,
    "taskParams" : {"restoreType" : "normalRestore"},
    "copies" :[{
        "protectSubObject" :[{"name" : "/liuxiaoxiang/happyFolder"}],
    }],
    "taskType" : 2,
    "extendInfo" : {
        "restoreCopyId": "sdaweaweawe",
        "restoreScn": "23635"
    }
    })";
    stub.set(&PluginAnyTimeRestore::ComposeNewRepository, ComposeNewRepositorySuccess);
    Json::Value jobData;
    CJsonUtils::ConvertStringtoJson(strValue, jobData);
    std::vector<mp_string> mountPoint;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    mp_int32 iRet = anyTimeRestore.ComputerAnyTimeRestoreLogPath(jobData, mountPoint, stRep, jsonRep_new);
    EXPECT_EQ(MP_SUCCESS, iRet);
    iRet = anyTimeRestore.ParseRestoreMetaInfo(path);
    EXPECT_EQ(MP_SUCCESS, iRet);
    std::vector<std::string> dirVec;
    iRet = anyTimeRestore.GetSCNLogDirList(dirVec);
    EXPECT_EQ(MP_FAILED, iRet);
}

/*
 * 用例名称：组合copy
 * 前置条件：
 * check点：1、Mock log 2、主任务初始化成功
 */
TEST_F(PluginAnyTimeRestoreTest, GetSCNLogDirListFailed)
{
    DoLogTest();
    std::string path = "xxx";
    stub.set(&CMpFile::ReadFile, mockopenfile);
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    auto anyTimeRestore = PluginAnyTimeRestore();
        std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e",
    "requestId" : "742222ad-721e-4243-a410-1eab3691f9e3",
    "subTaskType" : null,
    "taskParams" : {"restoreType" : "normalRestore"},
    "copies" :[{
        "protectSubObject" :[{"name" : "/liuxiaoxiang/happyFolder"}],
    }],
    "taskType" : 2,
    "extendInfo" : {
        "restoreCopyId": "sdaweaweawe",
        "restoreScn": "23641"
    }
    })";
    stub.set(&PluginAnyTimeRestore::ComposeNewRepository, ComposeNewRepositorySuccess);
    Json::Value jobData;
    CJsonUtils::ConvertStringtoJson(strValue, jobData);
    std::vector<mp_string> mountPoint;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    mp_int32 iRet = anyTimeRestore.ComputerAnyTimeRestoreLogPath(jobData, mountPoint, stRep, jsonRep_new);
    EXPECT_EQ(MP_SUCCESS, iRet);
    iRet = anyTimeRestore.ParseRestoreMetaInfo(path);
    EXPECT_EQ(MP_SUCCESS, iRet);
    std::vector<std::string> dirVec;
    iRet = anyTimeRestore.GetSCNLogDirList(dirVec);
    EXPECT_EQ(MP_FAILED, iRet);
}

/*
 * 用例名称：组合copy
 * 前置条件：
 * check点：1、Mock log 2、主任务初始化成功
 */
TEST_F(PluginAnyTimeRestoreTest, ComposeNewRepositoryTest)
{
    DoLogTest();
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    std::string  Repositorys= "[{\"auth\":{\"authKey\":\"admin\",\"authPwd\":\"Admin@123\",\"authType\":2,\"extendInfo\":null},\"endpoint\":{\"id\":\"\",\"ip\":\"2016:8:40:96:c11::105,8.40.102.106,2016:8:40:96:c11::106,8.40.102.105\",\"port\":8088},\"extendInfo\":null,\"isLocal\":true,\"path\":[\"/mnt/databackup/HBaseBackupSet/c61aa35e-234f-4dbb-9784-9a52160b490b/log/HBASE_e412cc6f-5ee5-48fa-8857-3f6513f23386_LogRepository/1643246242-1643336086/192.168.105.50\"],\"protocol\":1,\"remoteHost\":[{\"id\":null,\"ip\":\"8.42.102.106\",\"port\":null},{\"id\":null,\"ip\":\"192.168.105.50\",\"port\":null},{\"id\":null,\"ip\":\"8.42.102.105\",\"port\":null}],\"remotePath\":\"/HBASE_e412cc6f-5ee5-48fa-8857-3f6513f23386_LogRepository/1643246242-1643336086\",\"type\":3}]";

    auto anyTimeRestore = PluginAnyTimeRestore();
    Json::Value jobData;
    Json::Value newRepositories;
    CJsonUtils::ConvertStringtoJson(Repositorys, jobData);
    std::string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e",
    "requestId" : "742222ad-721e-4243-a410-1eab3691f9e3",
    "subTaskType" : null,
    "taskParams" : {"restoreType" : "normalRestore"},
    "copies" :[{
        "protectSubObject" :[{"name" : "/liuxiaoxiang/happyFolder"}],
    }],
    "taskType" : 2,
    "extendInfo" : {
        "restoreCopyId": "sdaweaweawe",
        "restoreTimestamp": "23637"
    }
    })";
    stub.set(&PluginAnyTimeRestore::ComposeNewRepository, ComposeNewRepositorySuccess);
    Json::Value cjobData;
    CJsonUtils::ConvertStringtoJson(strValue, cjobData);
    std::vector<mp_string> mountPoint;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    mp_int32 iRet = anyTimeRestore.ComputerAnyTimeRestoreLogPath(cjobData, mountPoint, stRep, jsonRep_new);
    EXPECT_EQ(MP_SUCCESS, iRet);
    stub.set(ADDR(PluginAnyTimeRestore, GetTimeStampLogDirList), mockGetTimeStampLogDirList);
    stub.set(ADDR(PluginAnyTimeRestore, ParseRestoreMetaInfo), mockParseRestoreMetaInfo);
    iRet = anyTimeRestore.ComposeNewRepository(mountPoint, stRep, jsonRep_new);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(PluginAnyTimeRestoreTest, ParseRestoreMetaInfo)
{
    DoLogTest();
    PluginAnyTimeRestore anyTimeRestore = PluginAnyTimeRestore();
    mp_string metapath;
    EXPECT_EQ(MP_FAILED, anyTimeRestore.ParseRestoreMetaInfo(metapath));

    metapath = "metapath";
    stub.set(ADDR(CRootCaller, Exec), StubFailed);
    EXPECT_EQ(MP_FAILED, anyTimeRestore.ParseRestoreMetaInfo(metapath));

    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    anyTimeRestore.m_restoreType = 1;
    EXPECT_EQ(MP_SUCCESS, anyTimeRestore.ParseRestoreMetaInfo(metapath));
    anyTimeRestore.m_restoreType = 2;
    EXPECT_EQ(MP_SUCCESS, anyTimeRestore.ParseRestoreMetaInfo(metapath));
}

TEST_F(PluginAnyTimeRestoreTest, GetTimeStampLogDirList)
{
    DoLogTest();
    PluginAnyTimeRestore anyTimeRestore = PluginAnyTimeRestore();

    std::vector<mp_string> logdirlist;
    EXPECT_EQ(MP_FAILED, anyTimeRestore.GetTimeStampLogDirList(logdirlist));

    Json::Value logjson;
    logjson["dirname"] = "dirname";
    logjson["starttime"] = 1;
    logjson["endtime"] = 9;

    anyTimeRestore.m_timeMultiMap.clear();
    anyTimeRestore.m_timeMultiMap.insert(std::make_pair(0, logjson));
    anyTimeRestore.m_restorePoint = 0;
    EXPECT_EQ(MP_FAILED, anyTimeRestore.GetTimeStampLogDirList(logdirlist));

    anyTimeRestore.m_timeMultiMap.clear();
    anyTimeRestore.m_timeMultiMap.insert(std::make_pair(10, logjson));
    anyTimeRestore.m_restorePoint = 10;
    EXPECT_EQ(MP_FAILED, anyTimeRestore.GetTimeStampLogDirList(logdirlist));

    anyTimeRestore.m_timeMultiMap.clear();
    anyTimeRestore.m_timeMultiMap.insert(std::make_pair(1, logjson));
    anyTimeRestore.m_restorePoint = 1;
    EXPECT_EQ(MP_SUCCESS, anyTimeRestore.GetTimeStampLogDirList(logdirlist));
}

TEST_F(PluginAnyTimeRestoreTest, GetSCNLogDirList)
{
    DoLogTest();
    PluginAnyTimeRestore anyTimeRestore = PluginAnyTimeRestore();

    std::vector<mp_string> logdirlist;
    EXPECT_EQ(MP_FAILED, anyTimeRestore.GetSCNLogDirList(logdirlist));

    Json::Value logjson;
    logjson["dirname"] = "dirname";
    logjson["startscn"] = 1;
    logjson["endscn"] = 9;

    anyTimeRestore.m_scnMultiMap.clear();
    anyTimeRestore.m_scnMultiMap.insert(std::make_pair(0, logjson));
    anyTimeRestore.m_restorePoint = 0;
    EXPECT_EQ(MP_FAILED, anyTimeRestore.GetSCNLogDirList(logdirlist));

    anyTimeRestore.m_scnMultiMap.clear();
    anyTimeRestore.m_scnMultiMap.insert(std::make_pair(10, logjson));
    anyTimeRestore.m_restorePoint = 10;
    EXPECT_EQ(MP_FAILED, anyTimeRestore.GetSCNLogDirList(logdirlist));

    anyTimeRestore.m_scnMultiMap.clear();
    anyTimeRestore.m_scnMultiMap.insert(std::make_pair(1, logjson));
    anyTimeRestore.m_restorePoint = 1;
    EXPECT_EQ(MP_SUCCESS, anyTimeRestore.GetSCNLogDirList(logdirlist));
}