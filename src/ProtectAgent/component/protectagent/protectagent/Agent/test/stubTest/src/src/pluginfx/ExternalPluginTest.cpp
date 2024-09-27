/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ExternalPluginTest.h
 * @brief  The implemention about ExternalPluginTest.h
 * @version 1.1.0.0
 * @date 2022-03-07
 * @author mwx1011302
 */
#include "pluginfx/ExternalPluginTest.h"
#include "pluginfx/ExternalPlugin.h"
#include "securecom/RootCaller.h"
#include "common/ConfigXmlParse.h"
#include "common/CSystemExec.h"
#include "common/Log.h"
#include "common/Ip.h"
#include "common/Path.h"
#include "stub.h"

namespace {
mp_void StubExPluginLogVoid(mp_void* pthis) {
    return;
}

mp_string StubGetExternalScriptPath(mp_string pluginName) {
    mp_string externalScriptPath = "/opt/DataBackup/ProtectClient/Plugins";
    return externalScriptPath;
}

mp_void StubSavePid(mp_string pluginName) {
    return;
}

mp_bool StubFileExistSucess(const mp_string& pszFilePath)
{
    return MP_TRUE;
}

mp_int32 StubExecSuccess(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_bool StubDelFileSucess(const mp_string& pszFilePath)
{
    return MP_TRUE;
}

mp_bool StubDirExistSuccess(const mp_string& pszFilePath)
{
    return MP_TRUE;
}

mp_bool StubDirExistFailed(const mp_string& pszFilePath)
{
    return MP_FALSE;
}

mp_bool StubIsPluginProcessExistSucc(const mp_string& pszFilePath)
{
    return MP_TRUE;
}

mp_bool StubIsPluginProcessExistFailed(const mp_string& pszFilePath)
{
    return MP_FALSE;
}

mp_int32 StubGenerateParamSucc(mp_string param)
{
    return MP_SUCCESS;
}

mp_int32 StubGetValueInt32Succ()
{
    return MP_SUCCESS;
}

mp_string StubGetExPluginSlogPath()
{
    mp_string getExPluginSlogPath = "/opt/DataBackup/ProtectClient/ProtectClient-E/slog";
    return getExPluginSlogPath;
}

mp_int32 StubExecSystemWithEcho0(const mp_string& strCommand, std::vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    mp_string gpuInfo1 = "VGA compatible controller: NVIDIA Corporation G98 Subsystem: NVIDIA Corporation Device 062e";
    mp_string gpuInfo2 = "Flags: bus master, fast devsel, latency 0, IRQ 24";
    mp_string gpuInfo3 = "Memory at f6000000 (32-bit, non-prefetchable) [size=16M]";
    mp_string gpuInfo4 = "Memory at ec000000 (64-bit, prefetchable) [size=64M]";
    mp_string gpuInfo5 = "Memory at f4000000 (64-bit, non-prefetchable) [size=32M]";
    mp_string gpuInfo6 = "I/O ports at dc80 [size=128]";
    mp_string gpuInfo7 = "[virtual] Expansion ROM at f7e00000 [disabled] [size=128K]";
    mp_string gpuInfo8 = "Capabilities: <access denied>";
    mp_string gpuInfo9 = "Kernel driver in use: nvidia";

    strEcho.push_back(gpuInfo1);
    strEcho.push_back(gpuInfo2);
    strEcho.push_back(gpuInfo3);
    strEcho.push_back(gpuInfo4);
    strEcho.push_back(gpuInfo5);
    strEcho.push_back(gpuInfo6);
    strEcho.push_back(gpuInfo7);
    strEcho.push_back(gpuInfo8);
    strEcho.push_back(gpuInfo9);
    return 0;
}
}

/*
*用例名称：改变外部插件状态
*前置条件：无
*check点：能否正常改变插件状态
*/
TEST_F(ExternalPluginTest, ChangeStatus) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginLogVoid);
    ExternalPlugin externalPlugin("HDFS", "test1", false);
    externalPlugin.ChangeStatus(EX_PLUGIN_STATUS::INITIALIZE);
}

/*
*用例名称：保存外部插件pid号
*前置条件：无
*check点：能否正常保存插件pid
*/
TEST_F(ExternalPluginTest, SavePid) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginLogVoid);
    ApplicationPlugin pluginInfo;
    pluginInfo.name = "NAS";
    pluginInfo.processId = "30012";
    mp_stub.set(ADDR(ExternalPlugin, GetExternalScriptPath), StubGetExternalScriptPath);
    
    ExternalPlugin externalPlugin("HDFS", "test1", false);
    externalPlugin.SavePid(pluginInfo);
}

/*
*用例名称：设置插件信息
*前置条件：无
*check点：能否设置插件信息
*/
TEST_F(ExternalPluginTest, SetPluginInfo) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginLogVoid);
    ApplicationPlugin pluginInfo;
    pluginInfo.name = "NAS";
    pluginInfo.processId = "30012";
    mp_stub.set(ADDR(ExternalPlugin, SavePid), StubSavePid);
    
    ExternalPlugin externalPlugin("HDFS", "test1", false);
    externalPlugin.SetPluginInfo(pluginInfo);
}

/*
*用例名称：执行停止插件
*前置条件：无
*check点：能否正常停插件
*/
TEST_F(ExternalPluginTest, ExecStopPlugin) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginLogVoid);
    ApplicationPlugin pluginInfo;
    pluginInfo.name = "NAS";
    pluginInfo.processId = "30012";

    mp_stub.set(ADDR(ExternalPlugin, GetExternalScriptPath), StubGetExternalScriptPath);
    mp_stub.set(ADDR(CMpFile, FileExist), StubFileExistSucess);
    mp_stub.set(ADDR(CRootCaller, ExecUserDefineScript), StubExecSuccess);
    mp_stub.set(ADDR(CMpFile, DelFile), StubDelFileSucess);

    ExternalPlugin externalPlugin("Fileset", "root", false);
    mp_int32 iRet = externalPlugin.ExecStopPlugin();
    EXPECT_EQ(MP_SUCCESS, iRet); 
}

/*
*用例名称：检查插件进程是否存在
*前置条件：无
*check点：插件进程判断是否正确
*/
TEST_F(ExternalPluginTest, IsPluginProcessExist) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginLogVoid);
    ExternalPlugin externalPlugin("HDFS", "test1", false);
    mp_stub.set(ADDR(CMpFile, DirExist), StubDirExistSuccess);
    mp_int32 iRet = externalPlugin.IsPluginProcessExist();
    EXPECT_EQ(iRet, MP_TRUE);

    mp_stub.set(ADDR(CMpFile, DirExist), StubDirExistFailed);
    iRet = externalPlugin.IsPluginProcessExist();
    EXPECT_EQ(iRet, MP_FALSE); 
}

/*
*用例名称：杀掉进程
*前置条件：无
*check点：是否成功杀掉进程
*/
TEST_F(ExternalPluginTest, KillProcess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginLogVoid);

    ExternalPlugin externalPlugin("HDFS", "test1", false);
    mp_stub.set(ADDR(ExternalPlugin, IsPluginProcessExist), StubIsPluginProcessExistSucc);
    mp_stub.set(ADDR(CRootCaller, Exec), StubExecSuccess);
    mp_stub.set(ADDR(CMpFile, DelFile), StubDelFileSucess);
    mp_int32 iRet = externalPlugin.KillProcess();
    EXPECT_EQ(MP_SUCCESS, iRet);

    mp_stub.set(ADDR(ExternalPlugin, IsPluginProcessExist), StubIsPluginProcessExistFailed);
    iRet = externalPlugin.KillProcess();
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
*用例名称：执行启动插件
*前置条件：无
*check点：执行是否成功
*/
TEST_F(ExternalPluginTest, ExecStartPlugin) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginLogVoid);

    ExternalPlugin externalPlugin("HDFS", "test1", false);
    mp_stub.set(ADDR(ExternalPlugin, GetExternalScriptPath), StubGetExternalScriptPath);
    mp_stub.set(ADDR(CMpFile, FileExist), StubFileExistSucess);
    mp_stub.set(ADDR(ExternalPlugin, GenerateParam), StubGenerateParamSucc);
    mp_stub.set(ADDR(CRootCaller, ExecUserDefineScript), StubExecSuccess);
    mp_stub.set(ADDR(CSystemExec,ExecSystemWithEcho), StubExecSystemWithEcho0);
    mp_int32 iRet = externalPlugin.ExecStartPlugin();
    EXPECT_EQ(MP_FAILED, iRet);
}

/*
*用例名称：生成参数
*前置条件：无
*check点：检查参数是否生成成功
*/
TEST_F(ExternalPluginTest, GenerateParam) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginLogVoid);
    mp_string params;
    ExternalPlugin externalPlugin("HDFS", "test1", false);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32Succ);
    mp_stub.set(ADDR(CPath,  GetExPluginSlogPath), StubGetExPluginSlogPath);
    mp_int32 iRet = externalPlugin.GenerateParam(params);
    EXPECT_EQ(MP_FAILED, iRet);
}

/*
*用例名称：调用插件任务后确定插件使用情况
*前置条件：增加、删除任务计数
*check点：检查插件是否可用状态
*/
TEST_F(ExternalPluginTest, PluginTaskNoUseTimeOut) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginLogVoid);
    ExternalPlugin externalPlugin("HDFS", "test1", false);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32Succ);
    mp_stub.set(ADDR(CPath,  GetExPluginSlogPath), StubGetExPluginSlogPath);

    bool isUse = externalPlugin.IsUsing();
    EXPECT_EQ(isUse, false);

    externalPlugin.AddTaskCounter();
    isUse = externalPlugin.IsUsing();
    EXPECT_EQ(isUse, true);

    externalPlugin.ReduceTaskCounter();
    isUse = externalPlugin.IsUsing();
    EXPECT_EQ(isUse, false);
    EXPECT_EQ(true, (std::chrono::steady_clock::now() - externalPlugin.m_noUsePoint) <= std::chrono::seconds(1));
}

/*
*用例名称：调用插件Restful后确定插件使用情况
*前置条件：增加、删除rest调用
*check点：检查插件是否可用状态
*/
TEST_F(ExternalPluginTest, PluginRestfulNoUseTimeOut) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginLogVoid);
    ExternalPlugin externalPlugin("HDFS", "test1", false);
    mp_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32Succ);
    mp_stub.set(ADDR(CPath,  GetExPluginSlogPath), StubGetExPluginSlogPath);

    bool isUse = externalPlugin.IsUsing();
    EXPECT_EQ(isUse, false);

    externalPlugin.AddRestCounter();
    isUse = externalPlugin.IsUsing();
    EXPECT_EQ(isUse, true);

    externalPlugin.ReduceRestCounter();
    isUse = externalPlugin.IsUsing();
    EXPECT_EQ(isUse, false);
    EXPECT_EQ(true, (std::chrono::steady_clock::now() - externalPlugin.m_noUsePoint) <= std::chrono::seconds(1));
}
