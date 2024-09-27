#include "apps/dws/XBSAServer/BsaTransState.h"
#include "apps/dws/XBSAServer/BsaSessionManager.h"
#include "apps/dws/XBSAServer/BsaTransManager.h"
#include "apps/dws/XBSAServer/BsaObjManager.h"

mp_int32 BsaTransStateBase::StateErr()
{
    ERRLOG("Bsa trans state error!bsaHandle=%lld,currentStatus=%u", m_trans->GetSessionId(), m_trans->GetCurStatus());
    return BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_CALL_SEQUENCE);
}

mp_int32 BsaTransStateBase::ChangeState(BsaTransStatus status)
{
    return m_trans->ChangeState(status);
}

mp_int32 BsaTransStateBase::DoCreateObj(BsaObjInfo &obj)
{
    if (!m_trans->IsLastOpSucc()) {
        ERRLOG("lastOp fail!bsaHandle=%lld,objectName(%s)", m_trans->GetSessionId(), obj.objectName.c_str());
        return BSA_RC_ABORT_SYSTEM_ERROR;
    }

    if ((BsaObjManager::GetInstance().CreateObject(obj) != MP_SUCCESS)) {
        ERRLOG("Create obj fail!bsaHandle=%lld,objectName(%s)", m_trans->GetSessionId(), obj.objectName.c_str());
        return BSA_RC_ABORT_SYSTEM_ERROR;
    }

    return ChangeState(BSA_TRANS_CREATE_OBJ);
}

mp_int32 BsaTransStateBase::DoSendData()
{
    // do nothing, just transfer to the next state.
    return ChangeState(BSA_TRANS_SEND_DATA);
}

mp_int32 BsaTransStateBase::DoEndData()
{
    // do nothing, just transfer to the next state.
    return ChangeState(BSA_TRANS_END_DATA);
}

mp_int32 BsaTransStateBase::DoDelObj(mp_uint64 copyId)
{
    // 1.2.1版本起XBSA对象通过统一调度框架整目录删除，正常情况下不会走XBSA删除流程.
    // XBSA协议接口中不再删除文件(也不支持删除,因为copyId不再唯一)，为保证协议完整性，这里直接按成功处理.
    BsaObjInfo obj;
    obj.copyId = copyId;
    m_trans->AddDelList(obj);

    return ChangeState(BSA_TRANS_DEL_OBJ);
}

mp_void BsaTransStateBase::GetNextQueryObj(BsaObjInfo &queryResult)
{
    queryResult = m_trans->GetQueryList().back();
    m_trans->GetQueryList().pop_back();
}

mp_int32 BsaTransStateBase::DoQueryObj(const BsaObjInfo &queryCond, BsaObjInfo &queryResult)
{
    if (!m_trans->IsLastOpSucc()) {
        ERRLOG("lastOp fail!bsaHandle=%lld,pathName(%s)", m_trans->GetSessionId(), queryCond.objectName.c_str());
        return BSA_RC_ABORT_SYSTEM_ERROR;
    }

    m_trans->InitQuery(queryCond);

    if (BsaObjManager::GetInstance().QueryObject(queryCond,
        m_trans->GetPageInfo(), m_trans->GetQueryList(), m_trans->GetSessionId())!= MP_SUCCESS) {
        ERRLOG("Query obj fail!bsaHandle=%lld,pathName(%s)", m_trans->GetSessionId(), queryCond.objectName.c_str());
        m_trans->SetLastOpRet(BSA_RC_ABORT_SYSTEM_ERROR);
        m_trans->GetQueryList().clear();
        return BsaSessionManager::GetInstance().SetLastErr(BSA_RC_ABORT_SYSTEM_ERROR);
    }

    if (m_trans->GetQueryList().empty()) {
        INFOLOG("DPP query 0 obj!bsaHandle=%lld,pathName(%s)", m_trans->GetSessionId(), queryCond.objectName.c_str());
        return BsaSessionManager::GetInstance().SetLastErr(BSA_RC_NO_MATCH);
    }

    m_trans->UpdatePageInfo(m_trans->GetQueryList().size());

    GetNextQueryObj(queryResult);
    return ChangeState(BSA_TRANS_QUERY_OBJ);
}

mp_int32 BsaTransStateBase::DoGetObj()
{
    // do nothing, just transfer to the next state.
    INFOLOG("Do get obj.");
    return ChangeState(BSA_TRANS_GET_OBJ);
}

mp_int32 BsaTransStateBase::DoGetNextObj(BsaObjInfo &queryResult)
{
    if (!m_trans->GetQueryList().empty()) {
        GetNextQueryObj(queryResult);
        return ChangeState(BSA_TRANS_GET_NEXT_OBJ);
    }

    if (!m_trans->NeedQueryNextPage()) {
        INFOLOG("no more objs!bsaHandle=%lld", m_trans->GetSessionId());
        return BsaSessionManager::GetInstance().SetLastErr(BSA_RC_NO_MORE_DATA);
    }

    if (BsaObjManager::GetInstance().QueryObject(m_trans->GetQueryCond(),
        m_trans->GetPageInfo(), m_trans->GetQueryList(), m_trans->GetSessionId())!= MP_SUCCESS) {
        ERRLOG("Query obj fail!bsaHandle=%lld,pathName(%s)",
            m_trans->GetSessionId(), m_trans->GetQueryCond().objectName.c_str());
        m_trans->SetLastOpRet(BSA_RC_ABORT_SYSTEM_ERROR);
        m_trans->GetQueryList().clear();
        return BsaSessionManager::GetInstance().SetLastErr(BSA_RC_ABORT_SYSTEM_ERROR);
    }

    if (m_trans->GetQueryList().empty()) {
        INFOLOG("no more objs!bsaHandle=%lld", m_trans->GetSessionId());
        return BsaSessionManager::GetInstance().SetLastErr(BSA_RC_NO_MORE_DATA);
    }

    m_trans->UpdatePageInfo(m_trans->GetQueryList().size());

    GetNextQueryObj(queryResult);
    return ChangeState(BSA_TRANS_GET_NEXT_OBJ);
}

mp_int32 BsaTransStateBase::DoGetData()
{
    // do nothing, just transfer to the next state.
    return ChangeState(BSA_TRANS_GET_DATA);
}

mp_int32 BsaTransStateIdle::CreateObj(BsaObjInfo &obj)
{
    return DoCreateObj(obj);
}

mp_int32 BsaTransStateIdle::QueryObj(const BsaObjInfo &queryCond, BsaObjInfo &queryResult)
{
    return DoQueryObj(queryCond, queryResult);
}

mp_int32 BsaTransStateIdle::GetObj()
{
    return DoGetObj();
}

mp_int32 BsaTransStateIdle::DelObj(mp_uint64 copyId)
{
    return DoDelObj(copyId);
}

mp_int32 BsaTransStateCreateObj::SendData()
{
    return DoSendData();
}

mp_int32 BsaTransStateCreateObj::EndData()
{
    return DoEndData();
}

mp_int32 BsaTransStateCreateObj::QueryObj(const BsaObjInfo &queryCond, BsaObjInfo &queryResult)
{
    return DoQueryObj(queryCond, queryResult);
}

mp_int32 BsaTransStateSendData::SendData()
{
    return DoSendData();
}

mp_int32 BsaTransStateSendData::EndData()
{
    return DoEndData();
}

mp_int32 BsaTransStateDelObj::DelObj(mp_uint64 copyId)
{
    return DoDelObj(copyId);
}

mp_int32 BsaTransStateDelObj::EndData()
{
    return DoEndData();
}

mp_int32 BsaTransStateDelObj::CreateObj(BsaObjInfo &obj)
{
    return DoCreateObj(obj);
}

mp_int32 BsaTransStateQueryObj::GetObj()
{
    return DoGetObj();
}

mp_int32 BsaTransStateQueryObj::QueryObj(const BsaObjInfo &queryCond, BsaObjInfo &queryResult)
{
    return DoQueryObj(queryCond, queryResult);
}

mp_int32 BsaTransStateQueryObj::GetNextObj(BsaObjInfo &queryResult)
{
    return DoGetNextObj(queryResult);
}

mp_int32 BsaTransStateQueryObj::DelObj(mp_uint64 copyId)
{
    return DoDelObj(copyId);
}

mp_int32 BsaTransStateQueryObj::EndData()
{
    return DoEndData();
}

mp_int32 BsaTransStateGetObj::GetObj()
{
    return DoGetObj();
}

mp_int32 BsaTransStateGetObj::GetData()
{
    return DoGetData();
}

mp_int32 BsaTransStateGetObj::GetNextObj(BsaObjInfo &queryResult)
{
    return DoGetNextObj(queryResult);
}

mp_int32 BsaTransStateGetObj::EndData()
{
    return DoEndData();
}

mp_int32 BsaTransStateGetNextObj::GetObj()
{
    return DoGetObj();
}

mp_int32 BsaTransStateGetNextObj::GetData()
{
    return DoGetData();
}

mp_int32 BsaTransStateGetNextObj::QueryObj(const BsaObjInfo &queryCond, BsaObjInfo &queryResult)
{
    return DoQueryObj(queryCond, queryResult);
}

mp_int32 BsaTransStateGetNextObj::GetNextObj(BsaObjInfo &queryResult)
{
    return DoGetNextObj(queryResult);
}

mp_int32 BsaTransStateGetNextObj::DelObj(mp_uint64 copyId)
{
    return DoDelObj(copyId);
}

mp_int32 BsaTransStateGetNextObj::EndData()
{
    return DoEndData();
}

mp_int32 BsaTransStateGetData::GetData()
{
    return DoGetData();
}

mp_int32 BsaTransStateGetData::EndData()
{
    return DoEndData();
}

mp_int32 BsaTransStateEndData::GetNextObj(BsaObjInfo &queryResult)
{
    return DoGetNextObj(queryResult);
}
