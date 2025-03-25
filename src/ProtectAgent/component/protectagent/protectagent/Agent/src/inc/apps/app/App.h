/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file App.h
 * @brief  Contains function declarations App
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
 */
// ------------------------------------------------------------------------------
// desc:   备份不区分应用类型插件相关接口定义，不考虑同一主机安装不同应用的场景，但需要支持同一主机安装同一应用
// 对于同一主机上相同应用的不同实例需要提供相同的应用鉴权的用户名密码。（当前实现SQLServer还不支持操作系统鉴权方式。）
// 的所有批量接口，在其中一个操作执行失败时则中断当前接口操作的执行，返回对应的失败信息。
// Copyright (C), 2015-2025, Huawei Tech. Co., Ltd.
// ------------------------------------------------------------------------------

#ifndef __AGENT_APP_H__
#define __AGENT_APP_H__

#include <vector>
#include <list>
#include "common/Types.h"
#include "common/CMpThread.h"
#include "common/Defines.h"

// 如果是windows上的vss操作，失败响应中的"dbName"字段的值固定填写"VSS"
static const mp_string DBNAME_FOR_VSS_IN_ERR_RESPONSE = "VSS";

typedef enum { APP_TYPE_ORACLE = 1, APP_TYPE_SQLSERVER, APP_TYPE_EXCHANGE, APP_TYPE_DB2, APP_TYPE_BUTT } APP_TYPE_E;

typedef struct tag_app_info {
    APP_TYPE_E enAppType;       // 应用类型
    mp_string strInstName;      // 实例名称
    mp_string strDBName;        // 数据库名(应用类型为Exchange的时候表示邮箱数据库)
    mp_string strVersion;       // 版本
    mp_string strStorageGroup;  // 存储组(仅Exchange 2007有效)
} app_info_t;

typedef struct tag_app_auth_info {
    mp_string strUserName;  // 应用鉴权用户名
    mp_string strPasswd;    // 应用鉴权密码
} app_auth_info_t;

typedef struct tag_app_failed_info {
    mp_int32 iErrorCode;  // 错误码
    mp_string strDbName;  // 数据名称；Windows上的VSS操作时固定填写"VSS"；
} app_failed_info_t;

class App {
public:
    App();
    ~App();

    mp_int32 Freeze(mp_time& tFreezeTime, std::vector<app_failed_info_t>& vecAppFailedList);
    mp_int32 UnFreeze(std::vector<app_failed_info_t>& vecAppFailedList);
    mp_int32 EndBackup(mp_int32 iBackupSucc, std::vector<app_failed_info_t>& vecAppFailedList);
    mp_int32 TruncateLog(mp_time tTruncateTime, std::vector<app_failed_info_t>& vecAppFailedList);
    mp_int32 QueryFreezeState(mp_int32& iState);
    mp_int32 UnFreezeEx();

private:
#ifdef WIN32
    VSSRequester* m_pVssRequester;
#endif
    mp_int32 m_freezeState;

    thread_lock_t m_freezeStateLock;

private:
    EXTER_ATTACK mp_int32 CheckFreezeIntervalTime(const mp_time& tFreezeTime,
        std::vector<app_failed_info_t>& vecAppFailedList);
    mp_int32 FreezeApp(std::vector<app_failed_info_t>& vecAppFailedList);
    mp_int32 UnFreezeApp(std::vector<app_failed_info_t>& vecAppFailedList);
    mp_int32 UnFreezeInner(std::vector<app_failed_info_t>& vecAppFailedList);
#ifdef WIN32
    mp_int32 FreezeVss(std::vector<app_failed_info_t>& vecAppFailedList);
    mp_int32 UnFreezeVss(std::vector<app_failed_info_t>& vecAppFailedLists);
    mp_int32 EndBackupVss(mp_int32 iBackupSucc, std::vector<app_failed_info_t>& vecAppFailedList);
    mp_void FreeVssRequester();
#endif
};

#endif  // __AGENT_APP_H__
