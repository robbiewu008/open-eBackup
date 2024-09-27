#include <sstream>
#include "common/Utils.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/ErrorCode.h"
#include "common/DB.h"
#include "securecom/CryptAlg.h"
#include "agent/FTExceptionHandle.h"

using namespace std;
FTExceptionHandle FTExceptionHandle::m_Instance;

/* ---------------------------------------------------------------------------
Function Name: HandleMonitorObjsProc
Description  : 线程处理函数，该线程处理监控对象列表中的数据
Others       :------------------------------------------------------------- */
#ifdef WIN32
DWORD WINAPI FTExceptionHandle::HandleMonitorObjsProc(LPVOID param)
#else
mp_void* FTExceptionHandle::HandleMonitorObjsProc(mp_void* param)
#endif
{
    FTExceptionHandle* pFtHandle = static_cast<FTExceptionHandle*>(param);

    pFtHandle->SetThreadStatus(THREAD_STATUS_RUNNING);
    while (!pFtHandle->NeedExit()) {
        pFtHandle->HandleMonitorObjs();
        DoSleep(SLEEP_TIME);
    }

    pFtHandle->SetThreadStatus(THREAD_STATUS_EXITED);
#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

mp_void FTExceptionHandle::SetThreadStatus(mp_int32 iThreadStatus)
{
    m_iThreadStatus = iThreadStatus;
}

mp_int32 FTExceptionHandle::GetThreadStatus()
{
    return m_iThreadStatus;
}

mp_bool FTExceptionHandle::NeedExit()
{
    return m_bNeedExit;
}

mp_void FTExceptionHandle::ProcessInternalRsps()
{
    mp_int32 iRet = MP_SUCCESS;
    message_pair_t stMsgPair;

    for (;;) {
        iRet = Communication::GetInstance().PopRspInternalMsgQueue(stMsgPair);
        if (iRet != 0) {
            break;
        }

        CRequestMsg *reqMsg = static_cast<CRequestMsg *>(stMsgPair.pReqMsg);
        CResponseMsg *rspMsg = static_cast<CResponseMsg *>(stMsgPair.pRspMsg);

        mp_bool bCheck = (reqMsg == NULL) || (rspMsg == NULL);
        if (bCheck) {
            COMMLOG(OS_LOG_ERROR, "convert message to request msg failed.");
            if (stMsgPair.pReqMsg != NULL) {
                delete stMsgPair.pReqMsg;
                stMsgPair.pReqMsg = NULL;
            }

            if (stMsgPair.pRspMsg != NULL) {
                delete stMsgPair.pRspMsg;
                stMsgPair.pRspMsg = NULL;
            }
            continue;
        }
        // 需要根据请求类型判断响应类型
        if (IsUnFreezeRequest(*reqMsg)) {
            (mp_void)ProccessUnFreezeRsp(*reqMsg, *rspMsg);
        } else if (IsQueryStatusRequest(*reqMsg)) {
            (mp_void)ProcessQueryStatusRsp(*reqMsg, *rspMsg);
        }

        // 内部请求不释放ReqMsg内部的FcgRequest和env内存，在删除监控对象时清理该内存
        // 此时这两块内存可能已经被监控对象超时或外部解冻请求执行成功删除监控对象操作释放
        if (stMsgPair.pReqMsg != NULL) {
            delete stMsgPair.pReqMsg;
            stMsgPair.pReqMsg = NULL;
        }

        if (stMsgPair.pRspMsg != NULL) {
            delete stMsgPair.pRspMsg;
            stMsgPair.pRspMsg = NULL;
        }
    }
}

mp_int32 FTExceptionHandle::ProccessUnFreezeRsp(CRequestMsg &pReqMsg, CResponseMsg &pRspMsg)
{
    mp_string strInstanceName;
    mp_bool bIsUnFreezeSucc = MP_FALSE;

    COMMLOG(OS_LOG_DEBUG, "Begin process unfreeze response.");
    mp_int32 ret = GetRequestInstanceName(pReqMsg, strInstanceName);
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get instance name failed, ret %d.", ret);
        return ret;
    }

    mp_string strDbName;
    ret = GetRequestDbName(pReqMsg, strDbName);
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get db name failed, ret %d.", ret);
        return ret;
    }

    MONITOR_OBJ* pMonitorObj = NULL;
    mp_int32 iAppType = GetRequestAppType(pReqMsg);
    pMonitorObj = GetMonitorObj(iAppType, strInstanceName, strDbName);
    if (pMonitorObj == NULL) {
        COMMLOG(OS_LOG_INFO, "Monitor obj dosen't exist.");
        return MP_SUCCESS;
    }

    mp_int32 iUnFreezeRet = (mp_int32)pRspMsg.GetRetCode();
    if (iUnFreezeRet == MP_SUCCESS) {
        bIsUnFreezeSucc = MP_TRUE;
    }
    ret = HandleUnFreezingMonitorObj(*pMonitorObj, bIsUnFreezeSucc);
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Handle unfreeze monitor obj failed, ret %d.", ret);
        return ret;
    }

    COMMLOG(OS_LOG_DEBUG, "Process unfreeze response succ.");
    return MP_SUCCESS;
}

mp_int32 FTExceptionHandle::ProcessQueryStatusRsp(CRequestMsg& pReqMsg, CResponseMsg& pRspMsg)
{
    mp_string strInstanceName;
    mp_int32 iQueryStatus = DB_UNKNOWN;

    COMMLOG(OS_LOG_DEBUG, "Begin query status response.");
    mp_int32 iRet = GetRequestInstanceName(pReqMsg, strInstanceName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get instance name failed, iRet %d.", iRet);
        return iRet;
    }

    mp_string strDbName;
    iRet = GetRequestDbName(pReqMsg, strDbName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get db name failed, iRet %d.", iRet);
        return iRet;
    }

    MONITOR_OBJ* pMonitorObj = NULL;
    mp_int32 iAppType = GetRequestAppType(pReqMsg);
    pMonitorObj = GetMonitorObj(iAppType, strInstanceName, strDbName);
    if (pMonitorObj == NULL) {
        COMMLOG(OS_LOG_INFO, "Monitor obj dosen't exist.");
        return MP_SUCCESS;
    }

    if (pRspMsg.GetRetCode() == MP_SUCCESS) {
        mp_int32 iTmpStatus = DB_UNKNOWN;
        iRet = CJsonUtils::GetJsonInt32(pRspMsg.GetJsonValueRef(), DB_FREEZE_STATUS, iTmpStatus);
        iQueryStatus = ((iRet == MP_SUCCESS) ? iTmpStatus : DB_UNKNOWN);
    }

    iRet = HandleQueryStatusMonitorObj(*pMonitorObj, iQueryStatus);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Handle query status monitor obj failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Process query status response succ.");
    return MP_SUCCESS;
}

mp_void FTExceptionHandle::HandleMonitorObjs()
{
    MONITOR_OBJ* pMonitorObj = NULL;
    CThreadAutoLock tmpLock(&m_tMonitorsLock);
    // 响应处理需要统一处理，在可能存在多个响应在队列中时，目前无法获取指定请求的响应
    // 接收处理发送的内部解冻、查询状态请求
    (mp_void)ProcessInternalRsps();

    // 遍历处理监控列表
    for (;;) {
        pMonitorObj = GetHandleMonitorObj();
        if (pMonitorObj == NULL) {
            break;
        }

        if (pMonitorObj->uiStatus == MONITOR_STATUS_FREEZED) {
            (mp_void)HandleFreezedMonitorObj(*pMonitorObj);
        }
    }
}

// 监控逻辑原则，始终监控最后一次冻结操作
mp_int32 FTExceptionHandle::HandleUnFreezingMonitorObj(MONITOR_OBJ& pMonitorObj, mp_bool bIsUnFreezeSucc)
{
    COMMLOG(OS_LOG_DEBUG, "Begin handle unfreezing monitor obj.");
    // 在下发查询请求到接收查询响应期间又有新的冻结请求下发，忽略该响应
    if (pMonitorObj.uiStatus == MONITOR_STATUS_FREEZED) {
        COMMLOG(OS_LOG_INFO, "The monitor obj was freezed again.");
        return MP_SUCCESS;
    }

    // 解冻失败上报告警，且修改状态为MONITOR_STATUS_FREEZED继续监控
    if (!bIsUnFreezeSucc) {
        COMMLOG(OS_LOG_ERROR, "Unfreeze monitor obj failed, send alarm.");
        SendHandleFailedAlarm(pMonitorObj);
        // 解冻失败延长下次监控解冻时间
        if (pMonitorObj.uiLoopTime <= MAX_MONITOR_TIME) {
            pMonitorObj.uiLoopTime = pMonitorObj.uiLoopTime * ftexceptionHandle_num_2;
        }
        pMonitorObj.uiStatus = MONITOR_STATUS_FREEZED;
        return MP_FAILED;
    }

    (mp_void)RemoveFromDB(pMonitorObj);
    DelMonitorObj(pMonitorObj);
    COMMLOG(OS_LOG_DEBUG, "Handle unfreezing monitor obj succ.");
    return MP_SUCCESS;
}

mp_int32 FTExceptionHandle::HandleQueryStatusMonitorObj(MONITOR_OBJ& pMonitorObj, mp_int32 iQueryStatus)
{
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, "Begin handle query status monitor obj.");
    // 在下发查询请求到接收查询响应期间又有新的冻结请求下发，忽略该响应
    if (pMonitorObj.uiStatus == MONITOR_STATUS_FREEZED) {
        COMMLOG(OS_LOG_INFO, "The monitor obj was freezed again.");
        return MP_SUCCESS;
    }

    // 解冻成功删除监控对象
    if (iQueryStatus == DB_UNFREEZE) {
        COMMLOG(OS_LOG_INFO, "The monitor obj has been unfreezed, delete it.");
        (mp_void)RemoveFromDB(pMonitorObj);
        DelMonitorObj(pMonitorObj);
    } else if (iQueryStatus == DB_FREEZE) {
        // 如果还是冻结状态则发送解冻命令，且更新状态为MONITOR_STATUS_UNFREEZING
        iRet = PushUnFreezeReq(pMonitorObj);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Push unfreeze request failed, iRet = %d.", iRet);
            pMonitorObj.uiStatus = MONITOR_STATUS_FREEZED;
            return iRet;
        } else {
            pMonitorObj.uiStatus = MONITOR_STATUS_UNFREEZING;
        }
    } else {
        // 如果查询状态失败则修改状态为MONITOR_STATUS_FREEZED继续监控
        COMMLOG(OS_LOG_INFO, "Get query status failed, monitor this obj again, iRet = %d.", iRet);
        pMonitorObj.uiStatus = MONITOR_STATUS_FREEZED;
    }

    COMMLOG(OS_LOG_DEBUG, "Handle query status monitor obj succ.");
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: ProccessFreezedMonitorObj
Description  : 处理待处理的监控对象
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::HandleFreezedMonitorObj(MONITOR_OBJ& pMonitorObj)
{
    mp_uint64 ulCurrentTime = CMpTime::GetTimeSec();
    // 冻结命令还没有响应，目前还处于延迟解冻期间，不进行处理
    if (ulCurrentTime <= pMonitorObj.ulBeginTime) {
        return MP_SUCCESS;
    }

    // 超过6小时，丢弃
    if (ulCurrentTime - pMonitorObj.ulBeginTime >= MAX_MONITOR_TIME) {
        (mp_void)RemoveFromDB(pMonitorObj);
        DelMonitorObj(pMonitorObj);
        return MP_SUCCESS;
    }

    // 还未到解冻时间则不进行处理
    if (ulCurrentTime - pMonitorObj.ulBeginTime < (mp_uint64)pMonitorObj.uiLoopTime) {
        return MP_SUCCESS;
    }
    
    mp_string strQueryUrl = GetQueryStatusUrl(pMonitorObj.iAppType);
    if (strQueryUrl == "") {
        COMMLOG(OS_LOG_ERROR, "Get query status url failed.");
        return MP_FAILED;
    }
    
    // 执行查询状态脚本时，如果脚本不存在，直接在PushQueryStatusReq内删除监控对象
    mp_int32 iRet = PushQueryStatusReq(pMonitorObj);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Push query status request failed, iRet %d.", iRet);
        return iRet;
    }

    pMonitorObj.uiStatus = MONITOR_STATUS_GETSTATUSING;
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: Init
Description  : 初始化函数，创建处理线程.
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::Init()
{
    CThreadAutoLock tmpLock(&m_tMonitorsLock);
    // 将持久化的监控对象加载到内存中
    mp_int32 iRet = LoadFromDB(m_vecMonitors);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Load monitor obj from database failed, iRet %d.", iRet);
        return iRet;
    }

    // 创建对象监控处理线程和定时线程
    iRet = CMpThread::Create(&m_hHandleThread, HandleMonitorObjsProc, this);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create ftexception handle thread failed, iRet %d.", iRet);
        return iRet;
    }

    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: WaitForExit
Description  : 等待线程退出
Others       :------------------------------------------------------------- */
mp_void FTExceptionHandle::WaitForExit()
{
    m_bNeedExit = MP_TRUE;
    while (m_iThreadStatus != THREAD_STATUS_EXITED) {
        DoSleep(ftexceptionHandle_num_100);
    }
}

/* ---------------------------------------------------------------------------
Function Name: MonitorFreezeOper
Description  : 处理冻结请求
Others       :------------------------------------------------------------- */
mp_void FTExceptionHandle::MonitorFreezeOper(CRequestMsg& pReqMsg)
{
    // 文件系统冻结包含多个对象，需要特殊处理
    if (IsFSRequest(pReqMsg)) {
        // 分拆json对象
        vector<mp_string> vecDisks;
        mp_int32 iRet = CJsonUtils::GetJsonArrayString(pReqMsg.GetMsgBody().GetJsonValueRef(), DISKNAMES, vecDisks);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR,  "Get json array value failed, key %s.", mp_string(DISKNAMES).c_str());
            return;
        }
        Json::Reader r;
        Json::Value jsRequest = pReqMsg.GetMsgBody().GetJsonValue();

        mp_string strJson;
        for (vector<mp_string>::iterator it = vecDisks.begin(); it != vecDisks.end(); ++it) {
            strJson = "{\"diskNames\":[\"" + *it + "\"]}";
            Json::Value jsValue;
            mp_bool bRet = r.parse(strJson.c_str(), jsValue);
            if (!bRet) {
                COMMLOG(OS_LOG_ERROR, "Parse json failed.");
                return;
            }
            pReqMsg.SetJsonData(jsValue);
            MonitorSingleFreezeOper(pReqMsg);
        }

        // 还原json对象
        pReqMsg.SetJsonData(jsRequest);
    } else {
        return MonitorSingleFreezeOper(pReqMsg);
    }
}

/* ---------------------------------------------------------------------------
Function Name: MonitorSingleFreezeOper
Description  : 处理单个冻结请求
Others       :------------------------------------------------------------- */
mp_void FTExceptionHandle::MonitorSingleFreezeOper(CRequestMsg& pReqMsg)
{
    MONITOR_OBJ monitorObj;

    if (!IsFreezeRequest(pReqMsg) && !IsUnFreezeRequest(pReqMsg)) {
        return;
    }

    COMMLOG(OS_LOG_DEBUG, "Begin monitor freeze oper.");
    if (IsFreezeRequest(pReqMsg)) {
        mp_int32 iRet = CreateMonitorObj(pReqMsg, monitorObj);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Create monitor obj failed, iRet = %d", iRet);
            return;
        }

        iRet = SaveToDB(monitorObj);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Save monitor obj to database failed, iRet = %d", iRet);
        }

        COMMLOG(OS_LOG_DEBUG, "Begin add monitor obj to list.");
        CThreadAutoLock tmpLock(&m_tMonitorsLock);
        iRet = AddMonitorObj(monitorObj);
        // 如果是监控列表已经有的对象，更新相关信息以后需要释放pReqMsg等动态内存
        if (iRet != MP_SUCCESS) {
            FreeMonitorObj(monitorObj);
        }

        COMMLOG(OS_LOG_INFO, "Add monitor obj succ.");
    }
    COMMLOG(OS_LOG_DEBUG, "Monitor freeze oper succ.");
}

/* ---------------------------------------------------------------------------
Function Name: CreateMonitorObj
Description  : 根据监控对象类型创建对应的监控对象
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::CreateMonitorObj(CRequestMsg& pReqMsg, MONITOR_OBJ& monitorObj)
{
    mp_int32 iRet = MP_SUCCESS;

    if (IsFSRequest(pReqMsg)) {
        iRet = CreateFSMonitorObj(pReqMsg, monitorObj);
    } else if (IsThirdPartyRequest(pReqMsg)) {
        iRet = CreateThirdPartyMonitorObj(pReqMsg, monitorObj);
    } else if (IsAppRequest(pReqMsg)) {
        iRet = CreateAppMonitorObj(pReqMsg, monitorObj);
    } else {
        COMMLOG(OS_LOG_ERROR,  "Unsupported monitor obj, url %s.", pReqMsg.GetURL().GetProcURL().c_str());
        return MP_FAILED;
    }
    return iRet;
}

/* ---------------------------------------------------------------------------
Function Name: CreateVSSMonitorObj
Description  : 创建VSS监控对象
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::CreateVSSMonitorObj(CRequestMsg& pReqMsg, MONITOR_OBJ& monitorObj)
{
    // VSS统一处理，这里不区分实例名和数据库名
    monitorObj.strInstanceName = VSS_INSTANCE_NAME;
    monitorObj.strDBName = VSS_DB_NAME;

    COMMLOG(OS_LOG_DEBUG, "Begin create vss monitor obj.");
    mp_int32 iRet = InitMonitorObj(monitorObj, pReqMsg);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init vss monitor obj failed, iRet = %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Create vss monitor obj succ.");
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CreateDBMonitorObj
Description  : 创建数据库监控对象
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::CreateDBMonitorObj(CRequestMsg& pReqMsg, MONITOR_OBJ& monitorObj)
{
    mp_string strInstanceNameForOtherDB = "";
    mp_string strInstanceNumForSAPHana = "";
    COMMLOG(OS_LOG_DEBUG, "Begin create db monitor obj.");
    mp_int32 iRet = CJsonUtils::GetJsonString(pReqMsg.GetMsgBody().GetJsonValueRef(),
        INST_NAME, strInstanceNameForOtherDB);
    mp_int32 iRetGetNum = CJsonUtils::GetJsonString(pReqMsg.GetMsgBody().GetJsonValueRef(),
        INST_NUM, strInstanceNumForSAPHana);
    if (iRet != MP_SUCCESS && iRetGetNum != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Get json string value failed, key1(%s) for sap hana, key2(%s) for other database.",
            mp_string(INST_NUM).c_str(),
            mp_string(INST_NAME).c_str());
        return iRet;
    }
    monitorObj.strInstanceName = ((iRet == MP_SUCCESS) ? strInstanceNameForOtherDB : strInstanceNumForSAPHana);
    iRet = CJsonUtils::GetJsonString(pReqMsg.GetMsgBody().GetJsonValueRef(), DBNAME, monitorObj.strDBName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get json string value failed, key %s.", mp_string(DBNAME).c_str());
        return iRet;
    }

    iRet = InitMonitorObj(monitorObj, pReqMsg);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init db2 or oracle monitor obj failed, iRet = %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Create db monitor obj succ.");
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CreateFSMonitorObj
Description  : 创建文件系统监控对象
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::CreateFSMonitorObj(CRequestMsg& pReqMsg, MONITOR_OBJ& monitorObj)
{
    vector<mp_string> vecDisks;
    mp_string strInstanceName;
    vector<mp_string>::iterator iter;

    COMMLOG(OS_LOG_DEBUG, "Begin create file system monitor obj.");
    mp_int32 iRet = CJsonUtils::GetJsonArrayString(pReqMsg.GetMsgBody().GetJsonValueRef(), DISKNAMES, vecDisks);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get json array value failed, key %s.", mp_string(DISKNAMES).c_str());
        return iRet;
    }

    for (iter = vecDisks.begin(); iter != vecDisks.end(); ++iter) {
        strInstanceName = strInstanceName + *iter + ":";
    }
    // 文件系统实例和数据库名统一使用挂载点或分区磁盘信息
    monitorObj.strInstanceName = std::move(strInstanceName);
    monitorObj.strDBName = FILESYTEM_DB_NAME;

    iRet = InitMonitorObj(monitorObj, pReqMsg);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init file system monitor obj failed, iRet = %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Create file system monitor obj succ.");
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CreateThirdPartyMonitorObj
Description  : 创建第三方脚本监控对象
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::CreateThirdPartyMonitorObj(CRequestMsg& pReqMsg, MONITOR_OBJ& monitorObj)
{
    mp_string strFileName;
    mp_string strParam;
    mp_int32 iRet = CJsonUtils::GetJsonString(
        pReqMsg.GetMsgBody().GetJsonValueRef(), REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME, strFileName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Get json string value failed, key %s.",
            REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME.c_str());
        return iRet;
    }
    GET_JSON_KEY_STRING(pReqMsg.GetMsgBody().GetJsonValueRef(), REST_PARAM_HOST_FREEZE_SCRIPT_PARAM, strParam);

    // 对于第三方冻结脚本，InstanceName使用脚本文件名称填充，DBName使用脚本参数填充
    monitorObj.strInstanceName = std::move(strFileName);
    monitorObj.strDBName = std::move(strParam);

    COMMLOG(OS_LOG_DEBUG, "Begin create third-party monitor obj.");
    iRet = InitMonitorObj(monitorObj, pReqMsg, mp_string(REST_PARAM_HOST_FREEZE_TIMEOUT));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init thirdparty monitor obj failed, iRet = %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Create thirdparty monitor obj succ.");
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CreateAppMonitorObj
Description  : 创建备份不区分应用接口监控对象
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::CreateAppMonitorObj(CRequestMsg& pReqMsg, MONITOR_OBJ& monitorObj)
{
    // 对于第三方冻结脚本，InstanceName使用脚本文件名称填充，DBName使用脚本参数填充
    monitorObj.strInstanceName = APPINSTNAME;
    monitorObj.strDBName = APPDBNAME;

    COMMLOG(OS_LOG_DEBUG, "Begin create app monitor obj.");
    mp_int32 iRet = InitMonitorObj(monitorObj, pReqMsg);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init app monitor obj failed, iRet = %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Create app monitor obj succ.");
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: UpdateFreezeOper
Description  : 更新监控对象状态
Others       :------------------------------------------------------------- */
mp_void FTExceptionHandle::UpdateFreezeOper(CRequestMsg& pReqMsg, CResponseMsg& pRspMsg)
{
    // 文件系统包含多个对象，需要特殊处理
    if (IsFSRequest(pReqMsg)) {
        // 分拆json对象
        vector<mp_string> vecDisks;
        mp_int32 iRet = CJsonUtils::GetJsonArrayString(pReqMsg.GetMsgBody().GetJsonValueRef(), DISKNAMES, vecDisks);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR,  "Get json array value failed, key %s.", mp_string(DISKNAMES).c_str());
            return;
        }
        Json::Reader r;
        Json::Value jVal = pReqMsg.GetMsgBody().GetJsonValue();
        mp_string strJson;
        for (vector<mp_string>::iterator iter = vecDisks.begin(); iter != vecDisks.end(); ++iter) {
            strJson = "{\"diskNames\":[\"" + *iter + "\"]}";
            Json::Value jsValue;
            mp_bool bRet = r.parse(strJson.c_str(), jsValue);
            if (!bRet) {
                COMMLOG(OS_LOG_ERROR, "Parse json failed.");
                return;
            }
            pReqMsg.SetJsonData(jsValue);
            UpdateSingleFreezeOper(pReqMsg, pRspMsg);
        }

        // 还原json对象
        pReqMsg.SetJsonData(jVal);
    } else {
        return UpdateSingleFreezeOper(pReqMsg, pRspMsg);
    }
}

/* ---------------------------------------------------------------------------
Function Name: UpdateSingleFreezeOper
Description  : 更新单个监控对象状态
Others       :------------------------------------------------------------- */
mp_void FTExceptionHandle::UpdateSingleFreezeOper(CRequestMsg& pReqMsg, CResponseMsg& pRspMsg)
{
    MONITOR_OBJ objCond;
    MONITOR_OBJ* pMonitorObj = NULL;

    if (!IsFreezeRequest(pReqMsg) && !IsUnFreezeRequest(pReqMsg)) {
        return;
    }

    mp_int32 iRet = CreateMonitorObj(pReqMsg, objCond);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create montior obj failed, iRet = %d.", iRet);
        return;
    }

    CThreadAutoLock tmpLock(&m_tMonitorsLock);
    pMonitorObj = GetMonitorObj(objCond);
    if (pMonitorObj == NULL) {
        COMMLOG(OS_LOG_INFO, "Monitor obj dosen't exist.");
        FreeMonitorObj(objCond);
        return;
    }

    COMMLOG(OS_LOG_DEBUG, "Begin update freeze operation monitor obj.");
    if (IsFreezeRequest(pReqMsg)) {
        if (pRspMsg.GetRetCode() == ERROR_COMMON_DB_USERPWD_WRONG) {
            COMMLOG(OS_LOG_DEBUG, "User or password wrong, begin free monitor obj resource.");
            (mp_void)RemoveFromDB(*pMonitorObj);
            DelMonitorObj(*pMonitorObj);
        } else {
            // 接收到冻结操作响应需要更新开始监控时间，监控对象监控开始时间从接收到冻结请求响应开始
            pMonitorObj->ulBeginTime = CMpTime::GetTimeSec();
            COMMLOG(OS_LOG_INFO,
                "Update monitor obj freeze begin time to %llu.",
                pMonitorObj->ulBeginTime);
        }
    } else {
        if (pRspMsg.GetRetCode() == MP_SUCCESS) {
            COMMLOG(OS_LOG_DEBUG, "Begin free monitor obj resource.");
            (mp_void)RemoveFromDB(*pMonitorObj);
            DelMonitorObj(*pMonitorObj);
        }
    }

    FreeMonitorObj(objCond);
    COMMLOG(OS_LOG_DEBUG, "Update freeze operation monitor obj succ.");
}

/* ---------------------------------------------------------------------------
Function Name:PushUnFreezeReq
Description  :解冻监控对象，构造内部请求放入通信请求队列
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::PushUnFreezeReq(MONITOR_OBJ& pMonitorObj)
{
    CRequestMsg* pReqMsg = NULL;
    CResponseMsg* pRspMsg = NULL;
    mp_string strUrl;

    strUrl = GetUnFreezeUrl(pMonitorObj.iAppType);
    if (strUrl == "") {
        COMMLOG(OS_LOG_ERROR, "Get unfreeze url failed.");
        return MP_FAILED;
    }
    // CodeDex误报，Memory Leak
    NEW_CATCH_RETURN_FAILED(pReqMsg, CRequestMsg);
    NEW_CATCH(pRspMsg, CResponseMsg);
    if (!pRspMsg) {
        delete pReqMsg;
        COMMLOG(OS_LOG_ERROR, "New response msg failed.");
        return MP_FAILED;
    }

    *pReqMsg = *(pMonitorObj.pReqMsg);
    message_pair_t stReqMsg(*pReqMsg, *pRspMsg);
    pReqMsg->GetURL().SetProcURL(strUrl);
    pReqMsg->GetHttpReq().SetMethod(REST_URL_METHOD_PUT);
    // pReqMsg和pRspMsg在内部响应处理完之后释放
    Communication::GetInstance().PushReqMsgQueue(stReqMsg);
    COMMLOG(OS_LOG_INFO, "Push unfreeze request succ, url %s.", pReqMsg->GetURL().GetProcURL().c_str());

    return MP_SUCCESS;
}

mp_int32 FTExceptionHandle::InitPushParam(MONITOR_OBJ& pMonitorObj, mp_string& strQueryParam)
{
    if (pMonitorObj.iAppType == TYPE_APP_FILESYSTEM) {
        strQueryParam = DISKNAMES + "=" + pMonitorObj.strInstanceName;
    } else if (pMonitorObj.iAppType == TYPE_APP_THIRDPARTY) {
        // 从json消息体中获取解冻脚本名称和参数
        Json::Value jv = pMonitorObj.pReqMsg->GetMsgBody().GetJsonValueRef();
        mp_string strStatusScript;
        mp_string strParm;
        mp_string isUserDefined;
        mp_string tempParm;
        GET_JSON_STRING(jv, REST_PARAM_HOST_QUERY_SCRIPT_FILENAME, strStatusScript);
        GET_JSON_STRING(jv, REST_ISUSERDEFINED_SCRIPT, isUserDefined);
        GET_JSON_KEY_STRING(jv, REST_PARAM_HOST_QUERY_SCRIPT_PARAM, tempParm);
        (mp_void)strParm.append(isUserDefined).append(":").append(tempParm);

        mp_string strScriptName = CPath::GetInstance().GetThirdPartyFilePath(strStatusScript, isUserDefined);
        if (!CMpFile::FileExist(strScriptName)) {
            // 脚本不存在删除监控对象，不需要上报告警，
            // 在执行冻结时脚本不存在直接会失败
            // 删除时不存在多线程问题，HandleMonitorObjs函数处理前会锁定monitor列表
            (mp_void)RemoveFromDB(pMonitorObj);
            DelMonitorObj(pMonitorObj);
            COMMLOG(OS_LOG_INFO, "QueryStatus file %s is not exists, delete monitor obj.", strScriptName.c_str());
            return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
        }
        strQueryParam = mp_string(REST_PARAM_HOST_QUERY_SCRIPT_FILENAME) + "=" + strStatusScript + "&" +
                        mp_string(REST_PARAM_HOST_QUERY_SCRIPT_PARAM) + "=" + strParm;
    } else if (pMonitorObj.iAppType == TYPE_APP_HANA) {
        strQueryParam = INST_NUM + "=" + pMonitorObj.strInstanceName + "&" + DBNAME + "=" + pMonitorObj.strDBName;
    } else {
        strQueryParam = INST_NAME + "=" + pMonitorObj.strInstanceName + "&" + DBNAME + "=" + pMonitorObj.strDBName;
    }

    return MP_SUCCESS;
}

mp_int32 FTExceptionHandle::PushQueryStatusReq(MONITOR_OBJ& pMonitorObj)
{
    // CodeDex误报，UNUSED_VALUE
    mp_string strQueryParam;
    CRequestMsg* pReqMsg = NULL;
    CResponseMsg* pRspMsg = NULL;

    mp_string strQueryUrl = GetQueryStatusUrl(pMonitorObj.iAppType);
    if (strQueryUrl == "") {
        COMMLOG(OS_LOG_ERROR, "Get query status url failed.");
        return MP_FAILED;
    }

    mp_int32 iRet = InitPushParam(pMonitorObj, strQueryParam);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    
    // CodeDex误报，Memory Leak
    NEW_CATCH(pReqMsg, CRequestMsg);
    if (!pReqMsg) {
        COMMLOG(OS_LOG_ERROR, "New CRequestMsg failed");
        return MP_FAILED;
    }

    NEW_CATCH(pRspMsg, CResponseMsg);
    if (!pRspMsg) {
        COMMLOG(OS_LOG_ERROR, "New CResponseMsg failed");
        delete pReqMsg;
        return MP_FAILED;
    }

    *pReqMsg = *(pMonitorObj.pReqMsg);
    message_pair_t stReqMsg(*pReqMsg, *pRspMsg);
    pReqMsg->GetURL().SetProcURL(strQueryUrl);
    if (pMonitorObj.iAppType != TYPE_APP_APP) {
        pReqMsg->GetURL().SetQueryParam(strQueryParam, MP_FALSE);
    }
    pReqMsg->GetHttpReq().SetMethod(REST_URL_METHOD_GET);
    // pReqMsg和pRspMsg在内部响应处理完之后释放
    Communication::GetInstance().PushReqMsgQueue(stReqMsg);
    COMMLOG(OS_LOG_INFO, "Push query status request succ, url %s.", pReqMsg->GetURL().GetProcURL().c_str());
    return MP_SUCCESS;
}

mp_bool FTExceptionHandle::IsExistInList(MONITOR_OBJ& monitorObj)
{
    vector<MONITOR_OBJ>::iterator iter;

    for (iter = m_vecMonitors.begin(); iter != m_vecMonitors.end(); ++iter) {
        if (IsSame(monitorObj, *iter)) {
            return MP_TRUE;
        }
    }

    return MP_FALSE;
}

/* ---------------------------------------------------------------------------
Function Name: GetHandleMonitorObj
Description  : 从监控列表中获取待处理的监控对象，如果已经遍历完一次则从第一个
               元素开始
Others       :------------------------------------------------------------- */
MONITOR_OBJ* FTExceptionHandle::GetHandleMonitorObj()
{
    if (m_vecMonitors.size() > 0 && m_iCurrIndex < m_vecMonitors.size()) {
        MONITOR_OBJ* pMonitorObj = NULL;
        pMonitorObj = static_cast<MONITOR_OBJ*>(&m_vecMonitors[m_iCurrIndex]);
        m_iCurrIndex++;
        return pMonitorObj;
    }

    m_iCurrIndex = 0;
    return NULL;
}

MONITOR_OBJ* FTExceptionHandle::GetMonitorObj(mp_int32 iAppType, mp_string& strInstanceName, mp_string& strDbName)
{
    MONITOR_OBJ* pMonitorObj = NULL;

    for (mp_uint32 uiIndex = 0; uiIndex < m_vecMonitors.size(); uiIndex++) {
        pMonitorObj = static_cast<MONITOR_OBJ*>(&m_vecMonitors[uiIndex]);
        if ((iAppType == pMonitorObj->iAppType) &&
            (strcmp(strInstanceName.c_str(), pMonitorObj->strInstanceName.c_str()) == 0) &&
            (strcmp(strDbName.c_str(), pMonitorObj->strDBName.c_str())) == 0) {
            return pMonitorObj;
        }
    }

    return NULL;
}

MONITOR_OBJ* FTExceptionHandle::GetMonitorObj(MONITOR_OBJ& monitorObj)
{
    return GetMonitorObj(monitorObj.iAppType, monitorObj.strInstanceName, monitorObj.strDBName);
}

mp_int32 FTExceptionHandle::AddMonitorObj(MONITOR_OBJ& monitorObj)
{
    // 对于新增的冻结监控对象BeginTime需要延迟，真正进行监控解冻的操作需要从冻结操作响应返回开始计时
    // 保存到数据库中的BeginTime不增加延迟时间，只对内存中的监控对象增加
    monitorObj.ulBeginTime += DELAY_UNFREEZE_TIME;

    if (IsExistInList(monitorObj)) {
        MONITOR_OBJ* pMonitorObj = NULL;
        pMonitorObj = GetMonitorObj(monitorObj);
        if (pMonitorObj != NULL) {
            pMonitorObj->uiStatus = monitorObj.uiStatus;
            pMonitorObj->ulBeginTime = monitorObj.ulBeginTime;
            pMonitorObj->uiLoopTime = monitorObj.uiLoopTime;
        }

        return MP_FAILED;
    }

    m_vecMonitors.push_back(monitorObj);
    return MP_SUCCESS;
}

mp_void FTExceptionHandle::AddMonitorObjs(vector<MONITOR_OBJ>& vecMonitorObjs)
{
    vector<MONITOR_OBJ>::iterator iter;

    for (iter = vecMonitorObjs.begin(); iter != vecMonitorObjs.end(); ++iter) {
        m_vecMonitors.push_back(*iter);
    }
}

mp_void FTExceptionHandle::DelMonitorObj(MONITOR_OBJ& pMonitorObj)
{
    vector<MONITOR_OBJ>::iterator iter;

    for (iter = m_vecMonitors.begin(); iter != m_vecMonitors.end();) {
        if (IsSame(pMonitorObj, *iter)) {
            FreeMonitorObj(*iter);
            m_vecMonitors.erase(iter);
            return;
        } else {
            ++iter;
        }
    }
}

/* ---------------------------------------------------------------------------
Function Name: DuplicateHead
Description  : 拷贝消息头里面的内容
Others       :------------------------------------------------------------- */
mp_char** FTExceptionHandle::DuplicateHead(const mp_string& strDbUser, const mp_string& strDbPp)
{
    mp_char** env = NULL;
    COMMLOG(OS_LOG_DEBUG, "Begin duplicate head.");
    // 多申请一个保存空地址，便于FCGX_GetParam计算结束标志
    // CodeDex误报，ZERO_LENGTH_ALLOCATIONS
    // CodeDex误报，Memory Leak
    NEW_ARRAY_CATCH(env, mp_char*, MAX_PRIVATE_KEY_NUM + 1);
    if (!env) {
        COMMLOG(OS_LOG_ERROR, "New env failed");
        return NULL;
    }

    mp_string strHeaderDbUser = mp_string(HTTPPARAM_DBUSERNAME) + "=" + strDbUser;
    NEW_ARRAY_CATCH(env[0], mp_char, strHeaderDbUser.length() + 1);
    if (!env[0]) {
        delete[] env;
        COMMLOG(OS_LOG_ERROR, "New env[0] failed");
        return NULL;
    }

    mp_int32 iRet = static_cast<mp_int32>(strcpy_s(env[0], strHeaderDbUser.length() + 1, strHeaderDbUser.c_str()));
    if (iRet != EOK) {
        delete[] env[0];
        delete[] env;
        COMMLOG(OS_LOG_ERROR, "Call strcpy_s failed, iRet = %d", iRet);
        return NULL;
    }

    mp_string strHeaderDbPp = mp_string(HTTPPARAM_DBPASSWORD) + "=" + strDbPp;
    NEW_ARRAY_CATCH(env[1], mp_char, strHeaderDbPp.length() + 1);
    if (!env[1]) {
        delete[] env[0];
        delete[] env;
        COMMLOG(OS_LOG_ERROR, "New env[1] failed");
        ClearString(strHeaderDbPp);
        return NULL;
    }
    env[ftexceptionHandle_num_2] = NULL;

    iRet = strcpy_s(env[1], strHeaderDbPp.length() + 1, strHeaderDbPp.c_str());
    if (iRet != EOK) {
        delete[] env[0];
        delete[] env[1];
        delete[] env;
        COMMLOG(OS_LOG_ERROR, "Call strcpy_s failed, iRet = %d", iRet);
        ClearString(strHeaderDbPp);
        return NULL;
    }

    COMMLOG(OS_LOG_DEBUG, "Duplicate head succ.");
    ClearString(strHeaderDbPp);
    return env;
}

mp_int32 FTExceptionHandle::CreateReqMsg(
    MONITOR_OBJ& monitorObj, mp_string& strDbUser, mp_string& strDbPp, const Json::Value& jvJsonData)
{
    FCGX_Request* pFcgxReq = NULL;

    COMMLOG(OS_LOG_DEBUG, "Begin create request msg.");
    // CodeDex误报，Memory Leak
    NEW_CATCH(monitorObj.pReqMsg, CRequestMsg);
    if (monitorObj.pReqMsg == NULL) {
        COMMLOG(OS_LOG_ERROR, "Create request msg for monitor obj failed.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    try {
        monitorObj.pReqMsg->SetJsonData(jvJsonData);
    }
    // SetJsonData方法中会引起Json::Value的复制操作，可能会调用JSON_ASSERT_MESSAGE宏抛出std::runtime_error异常
    // Coverity&Fortify，需要捕捉std::runtime_error异常
    catch (...) {
        delete monitorObj.pReqMsg;
        monitorObj.pReqMsg = NULL;
        COMMLOG(OS_LOG_ERROR, "JsonData is invalid.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_char** env = DuplicateHead(strDbUser, strDbPp);
    if (!env) {
        delete monitorObj.pReqMsg;
        monitorObj.pReqMsg = NULL;
        return ERROR_COMMON_INVALID_PARAM;
    }

    NEW_CATCH(pFcgxReq, FCGX_Request);
    if (!pFcgxReq) {
        delete[] env[0];
        delete[] env[1];
        delete[] env;
        delete monitorObj.pReqMsg;
        monitorObj.pReqMsg = NULL;
        COMMLOG(OS_LOG_ERROR, "New fcgx request failed.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    pFcgxReq->envp = env;
    monitorObj.pReqMsg->GetHttpReq().SetFcgxReq(*pFcgxReq);
    // 在monitor对象被释放时释放
    COMMLOG(OS_LOG_DEBUG, "Create request msg succ.");
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CreateReqMsg
Description  : 创建请求消息,供从数据库中读取出来构造请求信息时使用
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::CreateReqMsg(
    MONITOR_OBJ& monitorObj, mp_string& strEncryptDbUser, mp_string& strEncryptDbPp, mp_string& strJsonData)
{
    // CodeDex误报，UNUSED_VALUE
    Json::Reader jsonReader;
    Json::Value jsonValue;

    jsonReader = Json::Reader();
    try {
        if (!jsonReader.parse(strJsonData, jsonValue)) {
            COMMLOG(OS_LOG_ERROR, "JsonData is invalid.");
            return ERROR_COMMON_INVALID_PARAM;
        }
    }
    // jsonReader.parse方法中调用栈中可能会调用JSON_ASSERT_MESSAGE宏抛出std::runtime_error异常
    // Coverity&Fortify，需要捕捉std::runtime_error异常
    catch (...) {
        COMMLOG(OS_LOG_ERROR, "JsonData is invalid.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_string strDbUser;
    DecryptStr(strEncryptDbUser, strDbUser);
    mp_string strDbPp;
    DecryptStr(strEncryptDbPp, strDbPp);

    mp_int32 iRet = CreateReqMsg(monitorObj, strDbUser, strDbPp, jsonValue);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create request message failed, iRet %d.", iRet);
    }
    ClearString(strDbPp);
    return iRet;
}

mp_void FTExceptionHandle::FreeReqMsg(CRequestMsg& pReqMsg)
{
    // 释放内存前将内容清空
    (void)memset_s(pReqMsg.GetHttpReq().GetAllHead()[0],
        strlen(pReqMsg.GetHttpReq().GetAllHead()[0]),
        0,
        strlen(pReqMsg.GetHttpReq().GetAllHead()[0]));
    (void)memset_s(pReqMsg.GetHttpReq().GetAllHead()[1],
        strlen(pReqMsg.GetHttpReq().GetAllHead()[1]),
        0,
        strlen(pReqMsg.GetHttpReq().GetAllHead()[1]));

    delete[] pReqMsg.GetHttpReq().GetAllHead()[0];
    delete[] pReqMsg.GetHttpReq().GetAllHead()[1];
    delete[] pReqMsg.GetHttpReq().GetAllHead();
    delete pReqMsg.GetHttpReq().GetFcgxReq();
    delete (&pReqMsg);
}

mp_void FTExceptionHandle::FreeMonitorObj(MONITOR_OBJ& monitorObj)
{
    FreeReqMsg(*(monitorObj.pReqMsg));
    monitorObj.pReqMsg = NULL;
}

mp_int32 FTExceptionHandle::InitMonitorObj(MONITOR_OBJ& newMonitorObj,
    CRequestMsg& pReqMsg, const mp_string& LoopTimeKey)
{
    mp_string strDbUser;
    mp_string strDbPp;
    mp_int32 iLoopTime = 0;

    COMMLOG(OS_LOG_DEBUG, "Begin init monitor obj.");
    newMonitorObj.iAppType = GetRequestAppType(pReqMsg);
    newMonitorObj.ulBeginTime = CMpTime::GetTimeSec();
    newMonitorObj.uiStatus = MONITOR_STATUS_FREEZED;
    mp_int32 iRet = CJsonUtils::GetJsonInt32(pReqMsg.GetMsgBody().GetJsonValueRef(), LoopTimeKey, iLoopTime);
    if (iRet == MP_SUCCESS) {
        newMonitorObj.uiLoopTime = static_cast<mp_uint32>(iLoopTime);
    } else {
        newMonitorObj.uiLoopTime = THAW_WAIT_TIME;
    }

    strDbUser = pReqMsg.GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME);
    strDbPp = pReqMsg.GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD);
    iRet = CreateReqMsg(newMonitorObj, strDbUser, strDbPp, pReqMsg.GetMsgBody().GetJsonValueRef());
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create request msg for new monitor obj failed.");
    } else {
        COMMLOG(OS_LOG_DEBUG, "Init monitor obj succ.");
    }
    ClearString(strDbPp);
    return iRet;
}

/* ---------------------------------------------------------------------------
Function Name: IsExistInDB
Description  : 将监控数据持久化到数据库
Others       :------------------------------------------------------------- */
mp_bool FTExceptionHandle::IsExistInDB(MONITOR_OBJ& monitorObj)
{
    ostringstream buff;
    mp_string strSql;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    buff << "select " << INSTANCE_NAME << "," << DB_NAME << " from " << FREEZE_OBJ_TABLE << " where " << INSTANCE_NAME
        << " == ? and " << DB_NAME << " == ?";

    strSql = buff.str();
    COMMLOG(OS_LOG_DEBUG, "buff is = %s.", strSql.c_str());

    DbParamStream dps;
    DbParam dp = monitorObj.strInstanceName;
    dps << std::move(dp);
    dp = monitorObj.strDBName;
    dps << std::move(dp);

    mp_int32 iRet = CDB::GetInstance().QueryTable(strSql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query monitor object failed, iRet = %d.", iRet);
        return MP_FALSE;
    }

    if (iRowCount > 0) {
        return MP_TRUE;
    }

    return MP_FALSE;
}

/* ---------------------------------------------------------------------------
Function Name: SaveToDB
Description  : 将监控数据持久化到数据库
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::SaveToDB(MONITOR_OBJ& monitorObj)
{
    LOGGUARD("");

    if (IsExistInDB(monitorObj)) {
        COMMLOG(OS_LOG_INFO, "Monitor obj is aleady in db, instanceName is %s", monitorObj.strInstanceName.c_str());
        return MP_SUCCESS;
    }

    mp_string strDbUser = monitorObj.pReqMsg->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME);
    mp_string strDbPp = monitorObj.pReqMsg->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD);
    mp_string strEncrpytDBUser;
    EncryptStr(strDbUser, strEncrpytDBUser);
    mp_string strEncrpytDBPwd;
    EncryptStr(strDbPp, strEncrpytDBPwd);

    Json::FastWriter jfWriter;
    mp_string strJsonData = jfWriter.write(monitorObj.pReqMsg->GetMsgBody().GetJsonValueRef());
    std::ostringstream buff;
    buff << "insert into " << FREEZE_OBJ_TABLE << "(" << INSTANCE_NAME << "," << DB_NAME << "," << BEGIN_STATUS << ","
        << LOOPTIME << "," << USER << "," << MP << "," << JSON_DATA << "," << APPTYPE << "," << BEGIN_TIME
        << ") values(?, ?, ?, ?, ?, ?, ?, ?, ?);";
    mp_string sql = buff.str();

    DbParamStream dps;
    DbParam dp = monitorObj.strInstanceName;
    dps << std::move(dp);
    dp = monitorObj.strDBName;
    dps << std::move(dp);
    dp = monitorObj.uiStatus;
    dps << std::move(dp);
    dp = monitorObj.uiLoopTime;
    dps << std::move(dp);
    dp = strEncrpytDBUser;
    dps << std::move(dp);
    dp = strEncrpytDBPwd;
    dps << std::move(dp);
    dp = strJsonData;
    dps << std::move(dp);
    dp = monitorObj.iAppType;
    dps << std::move(dp);
    dp = monitorObj.ulBeginTime;
    dps << std::move(dp);

    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Execute sql failed,iRet = %d.", iRet);
    } else {
        COMMLOG(OS_LOG_INFO, "Save monitor obj succ, app type %d, isntance name %s, status %d, begin time %d.",
            monitorObj.iAppType, monitorObj.strInstanceName.c_str(), monitorObj.uiStatus, monitorObj.ulBeginTime);
    }
    (mp_void)strDbPp.replace(0, strDbPp.length(), "");
    return iRet;
}

/* ---------------------------------------------------------------------------
Function Name: RemoveFromDB
Description  : 将监控数据从数据库中移除
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::RemoveFromDB(MONITOR_OBJ& pMonitorObj)
{
    std::ostringstream buff;

    COMMLOG(OS_LOG_DEBUG, "Begin remove monitor obj from database.");

    buff << "delete from " << FREEZE_OBJ_TABLE << " where " << INSTANCE_NAME << "== ? and " << DB_NAME << " == ?";
    mp_string sql = buff.str();

    DbParamStream dps;
    DbParam dp = pMonitorObj.strInstanceName;
    dps << std::move(dp);
    dp = pMonitorObj.strDBName;
    dps << std::move(dp);

    mp_int32 iRet = CDB::GetInstance().ExecSql(sql, dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.ExecSql failed,iRet = %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO,
        "Remove monitor obj succ, app type %d, isntance name %s, "
        "status %d, begin time %d.",
        pMonitorObj.iAppType,
        pMonitorObj.strInstanceName.c_str(),
        pMonitorObj.uiStatus,
        pMonitorObj.ulBeginTime);
    COMMLOG(OS_LOG_DEBUG, "Remove monitor obj from database succ.");
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: LoadFromDB
Description  : 从db中读出监控数据，用于进程启动时调用
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::LoadFromDB(vector<MONITOR_OBJ>& vecObj)
{
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
    ostringstream buff;

    COMMLOG(OS_LOG_DEBUG, "Begin load monitor objs from database.");
    buff << "select " << INSTANCE_NAME << "," << DB_NAME << "," << BEGIN_STATUS << ","
        << LOOPTIME << "," << USER << "," << MP << "," << JSON_DATA << ","
        << APPTYPE << "," << BEGIN_TIME << " from " << FREEZE_OBJ_TABLE;

    DbParamStream dps;
    mp_int32 iRet = CDB::GetInstance().QueryTable(buff.str(), dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query freeze monitor obj table failed, iRet = %d.", iRet);
        return iRet;
    }

    for (mp_int32 iRow = 1; iRow <= iRowCount; ++iRow) {
        MONITOR_OBJ monitorObj;
        mp_string strInstanceName, strDBName, strStatus, strLoopTime;
        mp_string strUser, strPp, strJsonData, strAppType, strBeginTime;
        readBuff >> strInstanceName;
        readBuff >> strDBName;
        readBuff >> strStatus;
        readBuff >> strLoopTime;
        readBuff >> strUser;
        readBuff >> strPp;
        readBuff >> strJsonData;
        readBuff >> strAppType;
        readBuff >> strBeginTime;

        monitorObj.strInstanceName = strInstanceName;
        monitorObj.strDBName = std::move(strDBName);
        monitorObj.uiStatus = static_cast<mp_uint32>(atoi(strStatus.c_str()));
        monitorObj.uiLoopTime = static_cast<mp_uint32>(atoi(strLoopTime.c_str()));
        monitorObj.iAppType = atoi(strAppType.c_str());
        monitorObj.ulBeginTime = static_cast<mp_uint64>(atol(strBeginTime.c_str()));
        iRet = CreateReqMsg(monitorObj, strUser, strPp, strJsonData);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Generate request msg failed, iRet = %d", iRet);
        } else {
            COMMLOG(OS_LOG_INFO, "Load monitor obj succ, app type %d, isntance name %s, status %d, begin time %d.",
                monitorObj.iAppType, monitorObj.strInstanceName.c_str(), monitorObj.uiStatus, monitorObj.ulBeginTime);
            vecObj.push_back(monitorObj);
        }
    }

    COMMLOG(OS_LOG_DEBUG, "Load monitor objs from database succ.");
    return MP_SUCCESS;
}

mp_string FTExceptionHandle::GetDBNameFromObj(MONITOR_OBJ& pMonitorObj)
{
    mp_string strDBName = pMonitorObj.strDBName;
    if (pMonitorObj.pReqMsg && (pMonitorObj.iAppType == TYPE_APP_SQL) ||
        (pMonitorObj.iAppType == TYPE_APP_EXCHANGE)) {
        mp_int32 iRet = MP_FAILED;
        vector<Json::Value> vecJson;
        try {
            iRet = CJsonUtils::GetArrayJson(pMonitorObj.pReqMsg->GetMsgBody().GetJsonValue(), vecJson);
        } catch (...) {
            COMMLOG(OS_LOG_ERROR, "pMonitorObj is NULL.");
        }
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "GetArrayJson failed, iRet = %d.", iRet);
            return strDBName;
        }
        strDBName.clear();
        for (vector<Json::Value>::iterator it = vecJson.begin(); it != vecJson.end(); ++it) {
            mp_string strSingleDBName = "";
            iRet = CJsonUtils::GetJsonString(*it, DBNAME, strSingleDBName);
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "GetJsonString failed, iRet = %d.", iRet);
                return strDBName;
            }
            strDBName = strDBName + strSingleDBName + ":";
        }
    } else if (pMonitorObj.iAppType == TYPE_APP_FILESYSTEM) {
        strDBName = pMonitorObj.strInstanceName;
    }

    return strDBName;
}

/* ---------------------------------------------------------------------------
Function Name: SendHandleFailedAlarm
Description  : 发送解冻失败告警.
Others       :------------------------------------------------------------- */
mp_void FTExceptionHandle::SendHandleFailedAlarm(MONITOR_OBJ& pMonitorObj)
{
    COMMLOG(OS_LOG_DEBUG, "End send handle failed alarm.");
}

/* ---------------------------------------------------------------------------
Function Name:IsSame
Description  :判断两个监控对象是否相同
Others       :------------------------------------------------------------- */
mp_bool FTExceptionHandle::IsSame(MONITOR_OBJ& monitorObj1, MONITOR_OBJ& monitorObj2)
{
    mp_bool bFlag = (monitorObj1.iAppType == monitorObj2.iAppType) &&
                    (strcmp(monitorObj1.strInstanceName.c_str(), monitorObj2.strInstanceName.c_str()) == 0) &&
                    (strcmp(monitorObj1.strDBName.c_str(), monitorObj2.strDBName.c_str()) == 0);
    if (bFlag) {
        return MP_TRUE;
    }

    return MP_FALSE;
}

mp_bool FTExceptionHandle::IsFreezeRequest(CRequestMsg& pReqMsg)
{
    mp_string strUrl = pReqMsg.GetURL().GetProcURL();
    mp_bool bFlag = (strUrl == REST_DEVICE_FILESYS_FREEZE) ||
                    (strUrl == REST_HOST_FREEZE_SCRIPT) ||
                    (strUrl == REST_APP_FREEZE);
    if (bFlag) {
        return MP_TRUE;
    }

    return MP_FALSE;
}

mp_bool FTExceptionHandle::IsUnFreezeRequest(CRequestMsg& pReqMsg)
{
    mp_string strUrl = pReqMsg.GetURL().GetProcURL();
    mp_bool bFlag = (strUrl == REST_DEVICE_FILESYS_UNFREEZE) ||
                    (strUrl == REST_HOST_UNFREEZE_SCRIPT) ||
                    (strUrl == REST_APP_UNFREEZE) ||
                    (strUrl == REST_APP_UNFREEZEEX);
    if (bFlag) {
        return MP_TRUE;
    }

    return MP_FALSE;
}

/* ---------------------------------------------------------------------------
Function Name:IsFTFSRequest
Description  :根据url判断是否为文件系统冻结解冻请求
Others       :------------------------------------------------------------- */
mp_bool FTExceptionHandle::IsFSRequest(CRequestMsg& pReqMsg)
{
    mp_string strUrl = pReqMsg.GetURL().GetProcURL();
    mp_bool bFlag = (strUrl == REST_DEVICE_FILESYS_FREEZE) ||
                    (strUrl == REST_DEVICE_FILESYS_UNFREEZE) ||
                    (strUrl == REST_DEVICE_FILESYS_FREEZESTATUS);
    if (bFlag) {
        return MP_TRUE;
    }

    return MP_FALSE;
}

/* ---------------------------------------------------------------------------
Function Name:IsThirdPartyRequest
Description  :根据url判断是否为三方脚本冻结解冻请求
Others       :------------------------------------------------------------- */
mp_bool FTExceptionHandle::IsThirdPartyRequest(CRequestMsg& pReqMsg)
{
    mp_string strUrl = pReqMsg.GetURL().GetProcURL();
    mp_bool bFlag = (strUrl == REST_HOST_FREEZE_SCRIPT) ||
                    (strUrl == REST_HOST_UNFREEZE_SCRIPT) ||
                    (strUrl == REST_HOST_QUERY_STATUS_SCRIPT);
    if (bFlag) {
        return MP_TRUE;
    }

    return MP_FALSE;
}

/* ---------------------------------------------------------------------------
Function Name:IsThirdPartyRequest
Description  :根据url判断是否为备份的不感知应用冻结请求
Others       :------------------------------------------------------------- */
mp_bool FTExceptionHandle::IsAppRequest(CRequestMsg& pReqMsg)
{
    mp_string strUrl = pReqMsg.GetURL().GetProcURL();
    mp_bool bFlag = (strUrl == REST_APP_FREEZE) ||
                    (strUrl == REST_APP_UNFREEZE) ||
                    (strUrl == REST_APP_QUERY_DB_FREEZESTATE) ||
                    (strUrl == REST_APP_UNFREEZEEX);
    if (bFlag) {
        return MP_TRUE;
    }

    return MP_FALSE;
}

/* ---------------------------------------------------------------------------
Function Name:GetRequestAppType
Description  :根据url判断数据库类型
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::GetRequestAppType(CRequestMsg& pReqMsg)
{
    mp_string strUrl = pReqMsg.GetURL().GetProcURL();
    mp_int32 iAppType = TYPE_APP_UNKNOWN;

    if (mp_string::npos != strUrl.find(ORACLE)) {
        iAppType = TYPE_APP_ORACLE;
    } else if (mp_string::npos != strUrl.find(DB2)) {
        iAppType = TYPE_APP_DB2;
    } else if (mp_string::npos != strUrl.find(SQL)) {
        iAppType = TYPE_APP_SQL;
    } else if (mp_string::npos != strUrl.find(EXCHANGE)) {
        iAppType = TYPE_APP_EXCHANGE;
    } else if (mp_string::npos != strUrl.find(FILESYSTEM)) {
        iAppType = TYPE_APP_FILESYSTEM;
    } else if (mp_string::npos != strUrl.find(THIRDPARTY)) {
        iAppType = TYPE_APP_THIRDPARTY;
    } else if (mp_string::npos != strUrl.find(APP)) {
        iAppType = TYPE_APP_APP;
    } else if (mp_string::npos != strUrl.find(SYBASE)) {
        iAppType = TYPE_APP_SYBASE;
    } else if (mp_string::npos != strUrl.find(HANA)) {
        iAppType = TYPE_APP_HANA;
    }

    return iAppType;
}

mp_int32 FTExceptionHandle::GetRequestInstanceNameDB(CRequestMsg& pReqMsg, mp_string& strInstanceName)
{
    mp_string strInstanceNumForSAPHana = "";
    mp_string strInstanceNameForOtherDB = "";
    mp_int32 iRet = CJsonUtils::GetJsonString(pReqMsg.GetMsgBody().GetJsonValueRef(),
        INST_NAME, strInstanceNameForOtherDB);
    mp_int32 iRetGetNum =
        CJsonUtils::GetJsonString(pReqMsg.GetMsgBody().GetJsonValueRef(), INST_NUM, strInstanceNumForSAPHana);
    if (iRet != MP_SUCCESS && iRetGetNum != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Get json string value failed, key1(%s) for hana, key2(%s) for other db.",
            mp_string(INST_NUM).c_str(), mp_string(INST_NAME).c_str());
        return iRet;
    }
    strInstanceName = ((iRet == MP_SUCCESS) ? strInstanceNameForOtherDB : strInstanceNumForSAPHana);
    return MP_SUCCESS;
}

mp_int32 FTExceptionHandle::GetRequestInstanceNameFS(CRequestMsg& pReqMsg, mp_string& strInstanceName)
{
    mp_string strInstanceTmp;
    vector<mp_string> vecDisks;
    mp_int32 iRet = CJsonUtils::GetJsonArrayString(pReqMsg.GetMsgBody().GetJsonValueRef(), DISKNAMES, vecDisks);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,  "Get json array value failed, key %s.", mp_string(DISKNAMES).c_str());
        return iRet;
    }

    for (vector<mp_string>::iterator iter = vecDisks.begin(); iter != vecDisks.end(); ++iter) {
        strInstanceTmp = strInstanceTmp + *iter + ":";
    }
    strInstanceName = std::move(strInstanceTmp);
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name:GetRequestInstanceName
Description  :根据请求内容获取实例名称
Others       :------------------------------------------------------------- */
mp_int32 FTExceptionHandle::GetRequestInstanceName(CRequestMsg& pReqMsg, mp_string& strInstanceName)
{
    if (IsFSRequest(pReqMsg)) {
        return GetRequestInstanceNameFS(pReqMsg, strInstanceName);
    } else if (IsThirdPartyRequest(pReqMsg)) {
        mp_int32 iRet = CJsonUtils::GetJsonString(
            pReqMsg.GetMsgBody().GetJsonValueRef(), REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME, strInstanceName);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get json string value failed, key %s.",
                REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME.c_str());
            return iRet;
        }
    } else if (IsAppRequest(pReqMsg)) {
        strInstanceName = APPINSTNAME;
    } else {
        COMMLOG(OS_LOG_ERROR, "Unsupported monitor obj, url %s.", pReqMsg.GetURL().GetProcURL().c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 FTExceptionHandle::GetRequestDbName(CRequestMsg& pReqMsg, mp_string& strDbName)
{
    if (IsFSRequest(pReqMsg)) {
        strDbName = FILESYTEM_DB_NAME;
    } else if (IsThirdPartyRequest(pReqMsg)) {
        if (pReqMsg.GetMsgBody().GetJsonValueRef().isObject() &&
            pReqMsg.GetMsgBody().GetJsonValueRef().isMember(REST_PARAM_HOST_FREEZE_SCRIPT_PARAM)) {
            GET_JSON_KEY_STRING(
                pReqMsg.GetMsgBody().GetJsonValueRef(), REST_PARAM_HOST_FREEZE_SCRIPT_PARAM, strDbName);
        } else {
            strDbName = "";
        }
    } else if (IsAppRequest(pReqMsg)) {
        strDbName = APPDBNAME;
    } else {
        COMMLOG(OS_LOG_ERROR, "Unsupported monitor obj, url %s.", pReqMsg.GetURL().GetProcURL().c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name:ParseFTUrl
Description  :根据应用类型生成获取db状态url
Others       :------------------------------------------------------------- */
mp_string FTExceptionHandle::GetQueryStatusUrl(mp_int32 iAppType)
{
    if (queryRestUrls.find(iAppType) != queryRestUrls.end()) {
        return queryRestUrls[iAppType];
    } else {
        return "";
    }
}

mp_string FTExceptionHandle::GetUnFreezeUrl(mp_int32 iAppType)
{
    if (thawRestUrls.find(iAppType) != thawRestUrls.end()) {
        return thawRestUrls[iAppType];
    } else {
        return "";
    }
}

mp_bool FTExceptionHandle::IsQueryStatusRequest(CRequestMsg& pReqMsg)
{
    mp_string strUrl = pReqMsg.GetURL().GetProcURL();
    mp_bool bFlag =
        (strUrl == REST_DEVICE_FILESYS_FREEZESTATUS) ||
        (strUrl == REST_HOST_QUERY_STATUS_SCRIPT) ||
        (strUrl == REST_APP_QUERY_DB_FREEZESTATE);
    if (bFlag) {
        return MP_TRUE;
    }
    
    return MP_FALSE;
}
