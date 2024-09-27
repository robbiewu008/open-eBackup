/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ExternalPluginAlarmTest.h
 * @brief  The implemention about ExternalPluginAlarmTest.h
 * @version 1.0.0.0
 * @date 2021-12-17
 * @author jwx966562
 */
 #define private public
#include "pluginfx/ExternalPluginAlarmTest.h"
#include "pluginfx/ExternalPluginAlarmMng.h"
#include "pluginfx/ExternalPluginAlarmMng.h"
#include "common/Log.h"
#include "common/Ip.h"
#include "alarm/Trap.h"
#include "stub.h"
#include "host/host.h"

namespace {
mp_void CLogger_Log_Stub(mp_void* pthis) {
    return;
}

mp_void StubDoSleepTest100ms(mp_uint32 ms)
{
    struct timeval stTimeOut;
    const mp_int32 timeUnitChange = 1000;
    stTimeOut.tv_sec  = 100 / timeUnitChange;
    stTimeOut.tv_usec = (100 % timeUnitChange) * timeUnitChange;
    (mp_void)select(0, NULL, NULL, NULL, &stTimeOut);
}
}
mp_void StubExPluginManagerAlarmTestLogVoid(mp_void* pthis){
    return;
}

mp_int32 StubGetListenIPAndPortSuccess(void *obj, mp_string& strIP, mp_string& strPort)
{
    strIP = "127.0.0.1";
    return MP_SUCCESS;
}

mp_int32 StubGetListenIPAndPortFail(void *obj, mp_string& strIP, mp_string& strPort)
{
    return MP_FAILED;
}

mp_int32 StubAlarmSuccess(const alarm_param_t& alarmParam)
{
    return MP_SUCCESS;
}

mp_int32 StubAlarmFail(const alarm_param_t& alarmParam)
{
    return MP_FAILED;
}

mp_int32 StubGetHostSNSuccess(void *obj, mp_string& strSN)
{
    strSN = "123456";
    return MP_SUCCESS;
}
/*
*用例名称：上报插件启动失败告警成功
*前置条件：1、与PM通信正常 
*check点：能够正常上报告警
*/
TEST_F(ExternalPluginAlarmTest, broadcastAlarmSuccess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerAlarmTestLogVoid);
    mp_stub.set(ADDR(CIP, GetListenIPAndPort), StubGetListenIPAndPortSuccess);
    mp_stub.set(ADDR(AlarmHandle, Alarm), StubAlarmSuccess);
    mp_stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    ExternalPluginAlarmMng mng;
    mng.Init();
    mp_int32 ret = mng.BroadcastAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, "hdfs");
    EXPECT_EQ(ret, MP_SUCCESS);
}
/*
*用例名称：二次上报插件启动失败告警,不会重复告警
*前置条件：1、与PM通信正常 
*check点：二次上报插件启动失败告警,不会重复告警
*/
TEST_F(ExternalPluginAlarmTest, twiceBroadcastAlarm) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerAlarmTestLogVoid);
    mp_stub.set(ADDR(CIP, GetListenIPAndPort), StubGetListenIPAndPortSuccess);
    mp_stub.set(ADDR(AlarmHandle, Alarm), StubAlarmSuccess);
    mp_stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    ExternalPluginAlarmMng mng;
    mng.Init();
    mp_int32 ret = mng.BroadcastAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, "hdfs");
    EXPECT_EQ(ret, MP_SUCCESS);
    mng.BroadcastAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, "hdfs");
    EXPECT_EQ(ret, MP_SUCCESS);
}
/*
*用例名称：上报插件启动失败告警失败,因为获取agent使用的IP失败
*前置条件：1、无法获取agent使用的IP 
*check点：上报插件启动失败告警失败,因为获取agent使用的IP失败
*/
TEST_F(ExternalPluginAlarmTest, broadcastAlarmFail) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerAlarmTestLogVoid);
    mp_stub.set(ADDR(CIP, GetListenIPAndPort), StubGetListenIPAndPortFail);
    mp_stub.set(ADDR(AlarmHandle, Alarm), StubAlarmSuccess);
    mp_stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    ExternalPluginAlarmMng mng;
    mng.Init();
    mp_int32 ret = mng.BroadcastAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, "hdfs");
    EXPECT_EQ(ret, MP_FAILED);
}

/*
*用例名称：上报插件启动失败告警失败,因为与PM无法连接
*前置条件：1、无法获与PM通信
*check点：上报插件启动失败告警失败,因为与PM无法连接
*/
TEST_F(ExternalPluginAlarmTest, broadcastAlarmFailForPm) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerAlarmTestLogVoid);
    mp_stub.set(ADDR(CIP, GetListenIPAndPort), StubGetListenIPAndPortSuccess);
    mp_stub.set(ADDR(AlarmHandle, Alarm), StubAlarmFail);
    mp_stub.set(ADDR(CTrapSender, SendAlarm), StubAlarmFail);
    mp_stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    mp_stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    ExternalPluginAlarmMng mng;
    mng.Init();
    mp_int32 ret = mng.BroadcastAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, "hdfs");
    EXPECT_EQ(ret, MP_FAILED);
}
/*
*用例名称：上报插件启动失败恢复告警成功
*前置条件：1、已经上报过插件启动失败告警 
*check点：上报插件启动失败恢复告警成功
*/
TEST_F(ExternalPluginAlarmTest, ClearAlarmSuccess) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerAlarmTestLogVoid);
    mp_stub.set(ADDR(CIP, GetListenIPAndPort), StubGetListenIPAndPortSuccess);
    mp_stub.set(ADDR(AlarmHandle, Alarm), StubAlarmSuccess);
    mp_stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    mp_stub.set(ADDR(AlarmHandle, ClearAlarm), StubAlarmSuccess);
    ExternalPluginAlarmMng mng;
    mng.Init();
    mp_int32 ret = mng.BroadcastAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, "hdfs");
    EXPECT_EQ(ret, MP_SUCCESS);
    ret = mng.ClearAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, "hdfs");
    EXPECT_EQ(ret, MP_SUCCESS);
}
/*
*用例名称：上报插件启动失败恢复告警成功,因为没有上报过告警也需要上报了
*前置条件：1、没有上报过插件启动失败告警 
*check点：上报插件启动失败恢复告警成功,因为无论没有上报过告警都需要了
*/
TEST_F(ExternalPluginAlarmTest, ClearAlarmFail) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerAlarmTestLogVoid);
    mp_stub.set(ADDR(CIP, GetListenIPAndPort), StubGetListenIPAndPortSuccess);
    mp_stub.set(ADDR(AlarmHandle, Alarm), StubAlarmSuccess);
    mp_stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    mp_stub.set(ADDR(AlarmHandle, ClearAlarm), StubAlarmSuccess);
    ExternalPluginAlarmMng mng;
    mng.Init();
    mp_int32 ret = mng.ClearAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, "hdfs");
    EXPECT_EQ(ret, MP_SUCCESS);
}
/*
*用例名称：上报插件启动失败恢复告警失败,因为与PM无法连接
*前置条件：1、无法获与PM通信
*check点：上报插件启动失败恢复告警失败,因为与PM无法连接
*/
TEST_F(ExternalPluginAlarmTest, clearAlarmFailForPm) {
    Stub mp_stub;
    mp_stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, 
            const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubExPluginManagerAlarmTestLogVoid);
    mp_stub.set(ADDR(CIP, GetListenIPAndPort), StubGetListenIPAndPortSuccess);
    mp_stub.set(ADDR(AlarmHandle, Alarm), StubAlarmSuccess);
    mp_stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    mp_stub.set(ADDR(AlarmHandle, ClearAlarm), StubAlarmFail);
    mp_stub.set(ADDR(CTrapSender, ResumeAlarm), StubAlarmFail);
    ExternalPluginAlarmMng mng;
    mng.Init();
    mp_int32 ret = mng.BroadcastAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, "hdfs");
    EXPECT_EQ(ret, MP_SUCCESS);
    ret = mng.ClearAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, "hdfs");
    EXPECT_EQ(ret, MP_FAILED);
}