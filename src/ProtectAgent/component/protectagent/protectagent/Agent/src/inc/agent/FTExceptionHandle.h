/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file CFTExcetpionHandle.h
 * @brief  Contains function declarations for CFTExcetpionHandle
 * @version 0.1
 * @date 2015-01-19
 * @author yangwenjun 00275736
 */
#ifndef FT_EXCEPTION_HANDLE
#define FT_EXCEPTION_HANDLE

#include <vector>
#include "message/rest/message_process.h"
#include "common/CMpThread.h"
#include "agent/Communication.h"
#include "message/rest/interfaces.h"

static const mp_string INST_NAME = "instName";    // 数据库实例名称
static const mp_string INST_NUM = "instNum";      // HANA数据库的实例名是编号
static const mp_string APP_TYPE = "appType";      // 应用类型，参考APP_TYPE_E
static const mp_string FLUSH_TYPE = "frushType";  // 操作类型，参考FLUSH_TYPE_E
static const mp_string VSS_INSTANCE_NAME = "VSSinstName";
static const mp_string DBNAME = "dbName";
static const mp_string VSS_DB_NAME = "VSSdbName";
static const mp_string FILESYTEM_DB_NAME = "FiledbName";
static const mp_string DISKNAMES = "diskNames";
static const mp_string LOOP_TIME = "loop_time";
static const mp_string ERROR_CODE = "error";
static const mp_string DATA = "data";
static const mp_string DB_FREEZE_STATUS = "state";
static const mp_string ORACLE = "oracle";
static const mp_string DB2 = "db2";
static const mp_string SQL = "sql";
static const mp_string EXCHANGE = "exchange";
static const mp_string SYBASE = "sybase";
static const mp_string HANA = "hana";
static const mp_string FREEZE = "freeze";
static const mp_string UNFREEZE = "unfreeze";
static const mp_string FILESYSTEM = "filesystems";
static const mp_string THIRDPARTY = "thirdparty";
static const mp_string APP = "app";
static const mp_string APPDBNAME = "AppdbName";
static const mp_string APPINSTNAME = "AppInstName";

enum FLUSH_TYPE_E { FLUSH_TYPE_FREEZE = 0, FLUSH_TYPE_THAW };

enum APP_TYPE_E {
    TYPE_APP_DB2 = 0,
    TYPE_APP_ORACLE,
    TYPE_APP_SQL,
    TYPE_APP_EXCHANGE,
    TYPE_APP_FILESYSTEM,
    TYPE_APP_THIRDPARTY,
    TYPE_APP_APP,
    TYPE_APP_SYBASE,
    TYPE_APP_HANA,
    TYPE_APP_UNKNOWN
};

enum MONITOR_OBJ_STATUS_E {
    MONITOR_STATUS_FREEZED = 0,  // 冻结成功、冻结失败(渤海财险冻结接口返回失败实际后续成功)、未知状态统一由该状态表示
    MONITOR_STATUS_UNFREEZING,   // 已下发解冻请求
    MONITOR_STATUS_GETSTATUSING  // 已下发查询状态请求
};

typedef struct stMONITOR_OBJ {
    CRequestMsg* pReqMsg;   // 请求消息
    mp_uint32 uiStatus;     // 监控对象状态
    mp_uint64 ulBeginTime;  // 监控开始时间
    mp_string strInstanceName;
    mp_string strDBName;
    mp_int32 iAppType;
    mp_uint32 uiLoopTime;
} MONITOR_OBJ;

class FTExceptionHandle {
public:
    static FTExceptionHandle& GetInstance()
    {
        return m_Instance;
    }

    mp_int32 Init();
    mp_void WaitForExit();
    mp_void MonitorFreezeOper(CRequestMsg& pReqMsg);
    mp_void UpdateFreezeOper(CRequestMsg& pReqMsg, CResponseMsg& pRspMsg);
    ~FTExceptionHandle()
    {
        m_bNeedExit = MP_TRUE;
        if (m_hHandleThread.os_id != 0) {
            CMpThread::WaitForEnd(&m_hHandleThread, NULL);
        }
        CMpThread::DestroyLock(&m_tMonitorsLock);
    }

private:
    static FTExceptionHandle m_Instance;
    std::vector<MONITOR_OBJ> m_vecMonitors;  // 监控对象队列
    thread_lock_t m_tMonitorsLock;      // 监控队列锁
    thread_id_t m_hHandleThread;        // 保护线程id
    mp_uint32 m_iCurrIndex;             // 当前正在处理的监控对象索引
    volatile mp_int32 m_iThreadStatus;  // 监控线程状态
    volatile mp_bool m_bNeedExit;       // 线程退出标识

    static const mp_uchar ftexceptionHandle_num_100 = 100;
    static const mp_uchar ftexceptionHandle_num_2   = 2;
    static const mp_int32 MAX_MONITOR_TIME    = 21600;     // 放弃监控最长等待6小时,单位秒
    static const mp_int32 SLEEP_TIME          = 3000;     // 线程轮询处理时间，单位毫秒
    static const mp_uchar THAW_WAIT_TIME      = 60;           // 解冻等待时间
    static const mp_int32 MAX_GET_TIME        = 9000;         // 1分钟
    // 对于新加到监控列表中的冻结请求延迟10分钟解冻，oracle模块冻结操作目前是5分钟超时
    // (IO繁忙时冻结请求执行时间很长，正式统计解冻时间需要从冻结请求响应开始计算)
    static const mp_int32 DELAY_UNFREEZE_TIME = 600;
    static const mp_int32 MAX_PRIVATE_KEY_NUM = 2;

private:
    FTExceptionHandle()
    {
        (mp_void) memset_s(&m_hHandleThread, sizeof(m_hHandleThread), 0, sizeof(m_hHandleThread));
        // Coverity&Fortify误报:UNINIT_CTOR
        // Coveirty&Fortify不认识公司安全函数memset_s，提示m_dispatchTid.os_id未初始化
        CMpThread::InitLock(&m_tMonitorsLock);
        m_iThreadStatus = THREAD_STATUS_IDLE;
        m_bNeedExit = MP_FALSE;
        m_iCurrIndex = 0;

        queryRestUrls.insert(std::pair<mp_int32, mp_string>(TYPE_APP_FILESYSTEM, REST_DEVICE_FILESYS_FREEZESTATUS));
        queryRestUrls.insert(std::pair<mp_int32, mp_string>(TYPE_APP_THIRDPARTY, REST_HOST_QUERY_STATUS_SCRIPT));
        queryRestUrls.insert(std::pair<mp_int32, mp_string>(TYPE_APP_APP, REST_APP_QUERY_DB_FREEZESTATE));

        thawRestUrls.insert(std::pair<mp_int32, mp_string>(TYPE_APP_FILESYSTEM, REST_DEVICE_FILESYS_UNFREEZE));
        thawRestUrls.insert(std::pair<mp_int32, mp_string>(TYPE_APP_THIRDPARTY, REST_HOST_UNFREEZE_SCRIPT));
        thawRestUrls.insert(std::pair<mp_int32, mp_string>(TYPE_APP_APP, REST_APP_UNFREEZEEX));
    }

    std::map<mp_int32, mp_string> queryRestUrls;
    std::map<mp_int32, mp_string> thawRestUrls;

    // 监控线程回调函数
#ifdef WIN32
    static DWORD WINAPI HandleMonitorObjsProc(LPVOID param);
#else
    static mp_void* HandleMonitorObjsProc(mp_void* param);
#endif
    mp_void SetThreadStatus(mp_int32 iThreadStatus);
    mp_int32 GetThreadStatus();
    mp_bool NeedExit();
    // 处理对象队列中的对象
    mp_void ProcessInternalRsps();
    mp_int32 ProccessUnFreezeRsp(CRequestMsg& pReqMsg, CResponseMsg& pRspMsg);
    mp_int32 ProcessQueryStatusRsp(CRequestMsg& pReqMsg, CResponseMsg& pRspMsg);
    mp_void HandleMonitorObjs();
    mp_int32 HandleUnFreezingMonitorObj(MONITOR_OBJ& pMonitorObj, mp_bool bIsUnFreezeSucc);
    mp_int32 HandleQueryStatusMonitorObj(MONITOR_OBJ& pMonitorObj, mp_int32 iQueryStatus);
    mp_int32 HandleFreezedMonitorObj(MONITOR_OBJ& pMonitorObj);
    mp_int32 PushUnFreezeReq(MONITOR_OBJ& pMonitorObj);
    mp_int32 InitPushParam(MONITOR_OBJ& pMonitorObj, mp_string& strQueryParam);
    mp_int32 PushQueryStatusReq(MONITOR_OBJ& pMonitorObj);
    mp_void AddMonitorObjs(std::vector<MONITOR_OBJ>& vecMonitorObjs);
    mp_int32 AddMonitorObj(MONITOR_OBJ& monitorObj);
    MONITOR_OBJ* GetHandleMonitorObj();
    MONITOR_OBJ* GetMonitorObj(mp_int32 iAppType, mp_string& strInstanceName, mp_string& strDbName);
    MONITOR_OBJ* GetMonitorObj(MONITOR_OBJ& monitorObj);
    mp_bool IsExistInList(MONITOR_OBJ& monitorObj);
    mp_void DelMonitorObj(MONITOR_OBJ& pMonitorObj);
    mp_int32 CreateMonitorObj(CRequestMsg& pReqMsg, MONITOR_OBJ& monitorObj);
    mp_int32 CreateVSSMonitorObj(CRequestMsg& pReqMsg, MONITOR_OBJ& monitorObj);
    mp_int32 CreateDBMonitorObj(CRequestMsg& pReqMsg, MONITOR_OBJ& monitorObj);
    mp_int32 CreateFSMonitorObj(CRequestMsg& pReqMsg, MONITOR_OBJ& monitorObj);
    mp_int32 CreateThirdPartyMonitorObj(CRequestMsg& pReqMsg, MONITOR_OBJ& monitorObj);
    mp_int32 CreateAppMonitorObj(CRequestMsg& pReqMsg, MONITOR_OBJ& monitorObj);
    mp_char** DuplicateHead(const mp_string& strDbUser, const mp_string& strDbPp);
    mp_int32 CreateReqMsg(
        MONITOR_OBJ& monitorObj, mp_string& strDbUser, mp_string& strDbPp, const Json::Value& jvJsonData);
    mp_int32 CreateReqMsg(
        MONITOR_OBJ& monitorObj, mp_string& strEncryptDbUser, mp_string& strEncryptDbPp, mp_string& strJsonData);
    mp_void FreeReqMsg(CRequestMsg& pReqMsg);
    mp_void FreeMonitorObj(MONITOR_OBJ& monitorObj);
    mp_int32 InitMonitorObj(
        MONITOR_OBJ& newMonitorObj, CRequestMsg& pReqMsg, const mp_string& LoopTimeKey = mp_string(LOOP_TIME));
    mp_int32 SaveToDB(MONITOR_OBJ& monitorObj);
    mp_bool IsExistInDB(MONITOR_OBJ& monitorObj);
    mp_int32 RemoveFromDB(MONITOR_OBJ& pMonitorObj);
    mp_int32 LoadFromDB(std::vector<MONITOR_OBJ>& vecObj);
    mp_bool IsSame(MONITOR_OBJ& monitorObj1, MONITOR_OBJ& monitorObj2);
    mp_void SendHandleFailedAlarm(MONITOR_OBJ& pMonitorObj);
    mp_int32 GetRequestAppType(CRequestMsg& pReqMsg);
    mp_int32 GetRequestInstanceName(CRequestMsg& pReqMsg, mp_string& strInstanceName);
    mp_int32 GetRequestInstanceNameDB(CRequestMsg& pReqMsg, mp_string& strInstanceName);
    mp_int32 GetRequestInstanceNameFS(CRequestMsg& pReqMsg, mp_string& strInstanceName);
    mp_int32 GetRequestDbName(CRequestMsg& pReqMsg, mp_string& strDbName);
    mp_string GetQueryStatusUrl(mp_int32 iAppType);
    mp_string GetUnFreezeUrl(mp_int32 iAppType);
    mp_bool IsQueryStatusRequest(CRequestMsg& pReqMsg);
    mp_bool IsFreezeRequest(CRequestMsg& pReqMsg);
    mp_bool IsUnFreezeRequest(CRequestMsg& pReqMsg);
    mp_bool IsFSRequest(CRequestMsg& pReqMsg);
    mp_bool IsThirdPartyRequest(CRequestMsg& pReqMsg);
    mp_bool IsAppRequest(CRequestMsg& pReqMsg);

    mp_void MonitorSingleFreezeOper(CRequestMsg& pReqMsg);
    mp_void UpdateSingleFreezeOper(CRequestMsg& pReqMsg, CResponseMsg& pRspMsg);
    mp_string GetDBNameFromObj(MONITOR_OBJ& pMonitorObj);
};

#endif
