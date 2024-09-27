#include "apps/dws/XBSAServer/BsaSession.h"
#include "apps/dws/XBSAServer/BsaSessionManager.h"
#include "apps/dws/XBSAServer/BsaTransManager.h"
#include "apps/dws/XBSAServer/CTimer.h"
#include "apps/dws/XBSAServer/DwsTaskManage.h"
#include "apps/dws/XBSAServer/InformixTaskManage.h"
#include "apps/dws/XBSAServer/HcsTaskManage.h"
#include "apps/dws/XBSAServer/TpopsTaskManage.h"
#include "common/ConfigXmlParse.h"

namespace {
    const int MINUTE_TO_MILLISECONDS = 60 * 1000;

void SessionTimeOutCb(mp_uint64 bsaHandle, void *arg)
{
    ERRLOG("session CTimer timeout!close it!bsaHandle=%llu.", bsaHandle);
    (void)arg;

    (void)BsaSessionManager::GetInstance().SetLastErr(BSA_RC_TRANSACTION_ABORTED);
    (void)BsaSessionManager::GetInstance().CloseSession(bsaHandle);
}
}

mp_int32 BsaSession::GetSessionTimeoutInterval(mp_int32 &timeoutMs)
{
    mp_int32 timeout = -1;
    mp_int32 ret =
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_XBSA_SESSION_TIMEOUT_TIME, timeout);
    if (ret != MP_SUCCESS) {
        ERRLOG("get xbsa_session_timeout_time from agent_cfg.xml failed.ret=%d", ret);
        return MP_FAILED;
    }

    if (timeout <= 0 || timeout > (INT_MAX / MINUTE_TO_MILLISECONDS)) {
        ERRLOG("xbsa_session_timeout_time config invalid.timeout=%d.", timeout);
        return MP_FAILED;
    }
    timeoutMs = timeout * MINUTE_TO_MILLISECONDS;
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 BsaSession::Init(
    const BsaObjectOwner& objectOwner, const mp_string env[BSA_ENV_BUTT], mp_uint32 envSize)
{
    INFOLOG("This session's app type is %d", m_appType);
    if (CreateTaskManage() != MP_SUCCESS) {
        ERRLOG("Create task manage fail.");
        return MP_FAILED;
    }
    mp_int32 ret = strcpy_s(m_owner.bsa_ObjectOwner, BSA_MAX_BSAOBJECT_OWNER, objectOwner.bsaObjectOwner.c_str());
    if (ret != 0) {
        ERRLOG("strcpy_s bsaObjectOwner fail!bsaHandle=%lld,ret=%d", m_bsaHandle, ret);
        return MP_FAILED;
    }
    ret = strcpy_s(m_owner.app_ObjectOwner, BSA_MAX_APPOBJECT_OWNER, objectOwner.appObjectOwner.c_str());
    if (ret != 0) {
        ERRLOG("strcpy_s appObjectOwner fail!bsaHandle=%lld,ret=%d", m_bsaHandle, ret);
        return MP_FAILED;
    }
    mp_int32 timeoutMs = -1;
    ret = GetSessionTimeoutInterval(timeoutMs);
    if (ret != MP_SUCCESS || timeoutMs < 0) {
        ERRLOG("GetSessionTimeoutInterval fail!bsaHandle=%lld,ret=%d", m_bsaHandle, ret);
        return MP_FAILED;
    }
    for (mp_int32 i = 0; i < BSA_ENV_BUTT; i++) {
        SetEnv((BsaEnv)i, env[i]);
    }

    m_timerHandle = CTimer::GetInstance().StartTimer(timeoutMs, SessionTimeOutCb, m_bsaHandle, NULL);
    if (m_timerHandle == 0) {
        ERRLOG("StartTimer fail!bsaHandle=%lld", m_bsaHandle);
        return MP_FAILED;
    }

    INFOLOG("StartTimer succ.bsaHandle=%lld,timeoutMs=%dms,CTimer handle=%lu,cb=%p",
        m_bsaHandle, timeoutMs, m_timerHandle, SessionTimeOutCb);
    return MP_SUCCESS;
}

mp_int32 BsaSession::CreateTaskManage()
{
    LOGGUARD("");
    switch (m_appType) {
        case BSA_AppType::BSA_DWS: {
            m_appTaskManager = std::make_shared<DwsTaskManage>();
            INFOLOG("This session's app type is %d", m_appType);
            break;
        }
        case BSA_AppType::BSA_INFORMIX: {
            m_appTaskManager = std::make_shared<InformixTaskManage>();
            break;
        }
        case BSA_AppType::BSA_HCS: {
            m_appTaskManager = std::make_shared<HcsTaskManage>();
            INFOLOG("This session's app type is %d", m_appType);
            break;
        }
        case BSA_AppType::BSA_TPOPS: {
            m_appTaskManager = std::make_shared<TpopsTaskManage>();
            INFOLOG("This session's app type is %d", m_appType);
            break;
        }
        default: {
            ERRLOG("Do not support apptype %d", m_appType);
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_void BsaSession::Finit()
{
    if (m_timerHandle == 0) {
        ERRLOG("CTimer handle invalid!bsaHandle=%lld", m_bsaHandle);
        return;
    }
    CTimer::GetInstance().StopTimer(m_timerHandle);
}

mp_void BsaSession::SetEnv(BsaEnv idx, const mp_string &val)
{
    if (idx < BSA_ENV_BUTT) {
        if (val.empty()) {
            m_envs[idx] = "";
        } else {
            m_envs[idx] = BsaEnvKeys[idx] + "=" + val + ";";
        }
    }
}

mp_string BsaSession::GetEnv(BsaEnv idx)
{
    if (idx < BSA_ENV_BUTT) {
        return m_envs[idx];
    }
    return "";
}

mp_string BsaSession::GetAllEnv()
{
    mp_string envs = "";
    for (mp_int32 i = 0; i < BSA_ENV_BUTT; i++) {
        if (!m_envs[i].empty()) {
            envs += m_envs[i];
        }
    }
    return envs;
}

mp_bool inline BsaSession::TransExist()
{
    return (m_trans.get() ? MP_TRUE : MP_FALSE);
}

BsaTransaction *BsaSession::GetTrans()
{
    ResetTimer();
    return m_trans.get();
}

BSA_ObjectOwner &BsaSession::GetOwner()
{
    return m_owner;
}

mp_void BsaSession::ResetTimer()
{
    (mp_void)CTimer::GetInstance().ResetTimer(m_timerHandle);
}

mp_int32 BsaSession::BeginTxn()
{
    ResetTimer();

    if (TransExist()) {
        ERRLOG("Bsa trans already exist!bsaHandle=%lld", m_bsaHandle);
        return BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_CALL_SEQUENCE);
    }

    mp_long transId = BsaSessionManager::GetInstance().NewTransId();
    m_trans = std::make_shared<BsaTransaction>(m_bsaHandle, transId);
    if (!m_trans.get()) {
        ERRLOG("Bsa alloc trans failed!bsaHandle=%lld", m_bsaHandle);
        return BsaSessionManager::GetInstance().SetLastErr(BSA_RC_ABORT_SYSTEM_ERROR);
    }

    if (m_trans->BeginTxn() != MP_SUCCESS) {
        ERRLOG("Bsa trans begin fail!bsaHandle=%lld,transId=%lld", m_bsaHandle, transId);
        return BsaSessionManager::GetInstance().SetLastErr(BSA_RC_ABORT_SYSTEM_ERROR);
    }

    INFOLOG("begin Txn succ,bsaHandle=%lld,transId=%lld", m_bsaHandle, transId);
    return BSA_RC_SUCCESS;
}

mp_int32 BsaSession::EndTxn(BSA_Vote vote)
{
    ResetTimer();

    if (!TransExist()) {
        ERRLOG("trans not exist! bsaHandle=%lld", m_bsaHandle);
        return BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_CALL_SEQUENCE);
    }

    mp_int32 ret = m_trans->EndTxn(vote);
    if (ret != BSA_RC_SUCCESS) {
        ERRLOG("trans end fail! bsaHandle=%lld", m_bsaHandle);
        return BsaSessionManager::GetInstance().SetLastErr(ret);
    }
    
    INFOLOG("end Txn succ,bsaHandle=%lld,transId=%lld", m_bsaHandle, m_trans->GetTransId());
    m_trans.reset();
    return BSA_RC_SUCCESS;
}

mp_int32 BsaSession::UpdateTaskWhenCreateObject(const BsaObjectDescriptor &objDesc)
{
    if (m_appTaskManager.get() == nullptr) {
        ERRLOG("App task manager is null.");
        return MP_FAILED;
    }
    return m_appTaskManager->UpdateTaskWhenCreateObject(objDesc);
}
mp_int32 BsaSession::UpdateTaskWhenQueryObject(const BsaQueryDescriptor &objDesc)
{
    if (m_appTaskManager.get() == nullptr) {
        ERRLOG("App task manager is null.");
        return MP_FAILED;
    }
    return m_appTaskManager->UpdateTaskWhenQueryObject(objDesc);
}

mp_void BsaSession::AllocFilesystem(BsaObjInfo &objInfo)
{
    if (m_appTaskManager.get() == nullptr) {
        ERRLOG("App task manager is null.");
        return;
    }
    return m_appTaskManager->AllocFilesystem(objInfo);
}

mp_string BsaSession::GetTaskId()
{
    if (m_appTaskManager.get() == nullptr) {
        ERRLOG("App task manager is null.");
        return "";
    }
    return m_appTaskManager->GetTaskId();
}

mp_bool BsaSession::FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, QueryObjectResult &rsp)
{
    if (m_appTaskManager.get() == nullptr) {
        ERRLOG("App task manager is null.");
        return MP_FAILED;
    }
    return m_appTaskManager->FillQuryRsp(bsaHandle, queryReslt, rsp);
}
mp_bool BsaSession::FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, GetNextQueryObjectResult &rsp)
{
    if (m_appTaskManager.get() == nullptr) {
        ERRLOG("App task manager is null.");
        return MP_FAILED;
    }
    return m_appTaskManager->FillQuryRsp(bsaHandle, queryReslt, rsp);
}

const DwsCacheInfo &BsaSession::GetCacheInfo()
{
    if (m_appTaskManager.get() == nullptr) {
        ERRLOG("App task manager is null.");
        return DwsCacheInfo();
    }
    return m_appTaskManager->GetCacheInfo();
}

int32_t BsaSession::GetAppType()
{
    return m_appType;
}

void BsaSession::GenStorePath(BsaObjInfo &objInfo)
{
    if (m_appTaskManager.get() == nullptr) {
        ERRLOG("App task manager is null.");
        return;
    }
    return m_appTaskManager->GenStorePath(objInfo);
}