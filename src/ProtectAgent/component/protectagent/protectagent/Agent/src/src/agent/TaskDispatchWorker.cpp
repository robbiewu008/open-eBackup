#include "agent/TaskDispatchWorker.h"

#include <typeinfo>
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ErrorCode.h"
#include "message/Message.h"
#include "message/rest/interfaces.h"
#include "message/rest/message_process.h"

TaskDispatchWorker::TaskDispatchWorker()
{
    m_pWorkers = NULL;
    m_iWorkerCount = 0;
    m_lastPushedWorker = 0;
    (mp_void)memset_s(&m_dispatchTid, sizeof(m_dispatchTid), 0, sizeof(m_dispatchTid));
    // Coverity&Fortify误报:UNINIT_CTOR
    // Coveirty&Fortify不认识公司安全函数memset_s，提示m_dispatchTid.os_id未初始化
    m_bNeedExit = MP_FALSE;
    m_iThreadStatus = THREAD_STATUS_IDLE;
}

TaskDispatchWorker::~TaskDispatchWorker()
{
    m_pWorkers = NULL;
}

mp_bool TaskDispatchWorker::NeedExit()
{
    return m_bNeedExit;
}

mp_void TaskDispatchWorker::SetRecvThreadStatus(mp_int32 iThreadStatus)
{
    m_iThreadStatus = iThreadStatus;
}

/* ------------------------------------------------------------
Description  : 初始化分发线程
Input        : pTaskWorkers -- task worker线程数组的指针
               iCount -- task worker线程数组的个数
               pVssWorker -- vss worker线程指针
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
#ifdef WIN32
mp_int32 TaskDispatchWorker::Init(TaskWorker*& pTaskWorkers, mp_int32 iCount, TaskWorker& pVssWorker)
#else
mp_int32 TaskDispatchWorker::Init(TaskWorker*& pTaskWorkers, mp_int32 iCount)
#endif
{
    COMMLOG(OS_LOG_DEBUG, "Begin init task dispatch worker.");
    m_pWorkers = &pTaskWorkers;
#ifdef WIN32
    m_pVssWorker = &pVssWorker;
#endif
    m_iWorkerCount = iCount;
    mp_int32 iRet = CMpThread::Create(&m_dispatchTid, DispacthProc, this);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init task worker failed, ret %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "Init task dispatch worker succ.");

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 退出分发线程
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_void TaskDispatchWorker::Exit()
{
    m_bNeedExit = MP_TRUE;
    if (m_dispatchTid.os_id != 0) {
        CMpThread::WaitForEnd(&m_dispatchTid, NULL);
    }
}

/* ------------------------------------------------------------
Description  : 判断是否是VSS请求
Input        : msg -- 请求
Return       : MP_TRUE -- 是VSS请求
               MP_FALSE -- 不是VSS请求
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
#ifdef WIN32
mp_bool TaskDispatchWorker::IsVSSRequst(message_pair_t& msg)
{
    CRequestMsg *requestMsg = static_cast<CRequestMsg *>(msg.pReqMsg);
    if (requestMsg == NULL) {
        COMMLOG(OS_LOG_ERROR, "requestMsg is not CRequestMsg.");
        return MP_FALSE;
    }

    mp_string strUrl = requestMsg->GetURL().GetProcURL();
    COMMLOG(OS_LOG_DEBUG, "Request url is %s.", strUrl.c_str());
    mp_bool bFlag = REST_DEVICE_FILESYS_FREEZE == strUrl ||
                    REST_DEVICE_FILESYS_UNFREEZE == strUrl ||
                    REST_APP_FREEZE == strUrl ||
                    REST_APP_UNFREEZE == strUrl ||
                    REST_APP_UNFREEZEEX == strUrl ||
                    REST_APP_ENDBACKUP == strUrl;
    if (bFlag) {
        return MP_TRUE;
    }

    return MP_FALSE;
}
#endif

/* ------------------------------------------------------------
Description  : 传递消息给worker线程
Input        : msg -- 消息
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_void TaskDispatchWorker::PushMsgToWorker(message_pair_t& msg)
{
    mp_int32 iWorkIndex = 0;
    TaskWorker* pCurrWorker = NULL;
    mp_int32 m_firstPushedWorker = m_lastPushedWorker;
    mp_int32 count = 0;
#ifdef WIN32
    if (IsVSSRequst(msg)) {
        m_pVssWorker->PushRequest(msg);
        COMMLOG(OS_LOG_DEBUG, "Push vss request to vss worker succ.");
        return;
    }
#endif
    mp_bool bCheck = m_iWorkerCount <= 0 || m_pWorkers == NULL;
    if (bCheck) {
        COMMLOG(OS_LOG_ERROR, "Worker count is zero or worker pointer is null, can't dispatch request.");
        return;
    }

    do {
        iWorkIndex = m_lastPushedWorker % m_iWorkerCount;
        m_lastPushedWorker++;
        if (m_lastPushedWorker > MAX_PUSHED_WORKERS_CNT) {
            m_lastPushedWorker = 0;
        }
        pCurrWorker = *(m_pWorkers + iWorkIndex);
        if (((m_lastPushedWorker - m_firstPushedWorker) - 1) == m_iWorkerCount) {
            if (count++ < MAX_GET_WORKERS_COUNT) {
                m_firstPushedWorker = m_lastPushedWorker; // 更新first位置，开始新的一轮查找
                DoSleep(ALL_TASK_BUSY_WAIT_TIME);
                INFOLOG("Task workers are busy,the %d times try to find a idle worker.", count);
                continue;
            }
            COMMLOG(OS_LOG_INFO, "All task workers are busy.");
            return;
        }

        if (pCurrWorker->GetThreadProcReqStatus()) {
            COMMLOG(OS_LOG_DEBUG, "Task worker[%d] is busy.", iWorkIndex + 1);
        } else {
            COMMLOG(OS_LOG_DEBUG, "Task worker[%d] is idle.", iWorkIndex + 1);
        }
    } while (pCurrWorker->GetThreadProcReqStatus());
    pCurrWorker->PushRequest(msg);
    if (msg.pReqMsg->GetTypeID() == REQMESSAGE_TYPE) {
        Communication::GetInstance().PopReqMsgQueue(msg);
    } else {
        MessageHandler::GetInstance().PopReqMsg(msg);
    }
    COMMLOG(OS_LOG_DEBUG, "Push request to task worker[%d] succ.",
        iWorkIndex + 1);
}

/* ------------------------------------------------------------
Description  : 处理无空闲worker线程的情况
Input        : msg -- 消息
Create By    : caomin 00511255
------------------------------------------------------------- */
mp_void TaskDispatchWorker::HandleAllTaskBusy(message_pair_t& msg)
{
    COMMLOG(OS_LOG_DEBUG, "Enter HandleAllTaskBusy, %d", msg.pReqMsg->GetTypeID());
    if (msg.pReqMsg->GetTypeID() == REQMESSAGE_TYPE) {
        COMMLOG(OS_LOG_DEBUG, "Push ReqMsgQueue");
        Communication::GetInstance().PushReqMsgQueue(msg);
    } else {
        COMMLOG(OS_LOG_DEBUG, "Push ReqMsg");
        MessageHandler::GetInstance().PushReqMsg(msg);
    }
    COMMLOG(OS_LOG_ERROR, "All task workers are busy.");
}

/* ------------------------------------------------------------
Description  : 分发线程的线程回调函数
Input        : pThis -- 线程回调函数参数
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
#ifdef WIN32
DWORD WINAPI TaskDispatchWorker::DispacthProc(void* pThis)
#else
void* TaskDispatchWorker::DispacthProc(void* pThis)
#endif
{
    mp_int32 iRet = MP_SUCCESS;
    message_pair_t msg;
    TaskDispatchWorker* pDispathWorker = static_cast<TaskDispatchWorker*>(pThis);

    COMMLOG(OS_LOG_DEBUG, "Begin dispatch request.");
    pDispathWorker->SetRecvThreadStatus(THREAD_STATUS_RUNNING);
    while (!pDispathWorker->NeedExit()) {
        mp_bool found = MP_FALSE;
        // need to search rest and dpp message
        iRet = MessageHandler::GetInstance().GetFrontReqMsg(msg);
        if (iRet == MP_SUCCESS) {
            pDispathWorker->PushMsgToWorker(msg);
            found = MP_TRUE;
        }

        iRet = Communication::GetInstance().GetFrontReqMsgQueue(msg);
        if (iRet == MP_SUCCESS) {
            pDispathWorker->PushMsgToWorker(msg);
            found = MP_TRUE;
        }

        // if there is no message, continue
        if (found == MP_FALSE) {
            DoSleep(TASK_DISPATCH_WORKER_NUM_100);
            continue;
        }
    }
    COMMLOG(OS_LOG_DEBUG, "End dispatch request.");
    pDispathWorker->SetRecvThreadStatus(static_cast<mp_int32>(THREAD_STATUS_EXITED));
#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}
