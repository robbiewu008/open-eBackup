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
#include "apps/dws/XBSAServer/BsaTransManager.h"
#include "common/File.h"
#include "apps/dws/XBSAServer/BsaTransState.h"
#include "apps/dws/XBSAServer/BsaSessionManager.h"
#include "apps/dws/XBSAServer/BsaIntfAdaptor.h"
#include "apps/dws/XBSAServer/BsaObjManager.h"

namespace {
    const mp_int32 RET_BASE_NULL = BSA_RC_ABORT_SYSTEM_ERROR;
    const mp_uint32 BSA_QUERY_LIMIT = 50;
}

mp_int32 BsaTransaction::InitStateMachine()
{
    m_stateMachine.m_base = std::make_unique<BsaTransStateIdle>(this);
    if (!m_stateMachine.m_base.get()) {
        ERRLOG("Bsa trans state machine init fail!");
        return MP_ERROR;
    }

    m_stateMachine.m_curStatus = BSA_TRANS_IDLE;

    return MP_SUCCESS;
}

mp_int32 BsaTransaction::ChangeState(BsaTransStatus status)
{
    std::lock_guard<std::mutex> lock(m_stateMachine.m_statusMutex);
    m_stateMachine.m_base.reset();
    switch (status) {
        case BSA_TRANS_IDLE:
            m_stateMachine.m_base = std::make_unique<BsaTransStateIdle>(this);
            break;
        case BSA_TRANS_CREATE_OBJ:
            m_stateMachine.m_base = std::make_unique<BsaTransStateCreateObj>(this);
            break;
        case BSA_TRANS_SEND_DATA:
            m_stateMachine.m_base = std::make_unique<BsaTransStateSendData>(this);
            break;
        case BSA_TRANS_DEL_OBJ:
            m_stateMachine.m_base = std::make_unique<BsaTransStateDelObj>(this);
            break;
        case BSA_TRANS_QUERY_OBJ:
            m_stateMachine.m_base = std::make_unique<BsaTransStateQueryObj>(this);
            break;
        case BSA_TRANS_GET_OBJ:
            m_stateMachine.m_base = std::make_unique<BsaTransStateGetObj>(this);
            break;
        case BSA_TRANS_GET_NEXT_OBJ:
            m_stateMachine.m_base = std::make_unique<BsaTransStateGetNextObj>(this);
            break;
        case BSA_TRANS_GET_DATA:
            m_stateMachine.m_base = std::make_unique<BsaTransStateGetData>(this);
            break;
        case BSA_TRANS_END_DATA:
            m_stateMachine.m_base = std::make_unique<BsaTransStateEndData>(this);
            break;
        default:
            ERRLOG("Bsa trans change state error!status=%d,curStatus=%d", status, m_stateMachine.m_curStatus);
            return BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_CALL_SEQUENCE);
    }

    if (!m_stateMachine.m_base.get()) {
        ERRLOG("Bsa trans state machine alloc fail!");
        return RET_BASE_NULL;
    }

    m_stateMachine.m_curStatus = status;

    return BSA_RC_SUCCESS;
}

BsaTransaction::BsaTransaction(mp_long bsaHandle, mp_long transId)
{
    m_bsaHandle = bsaHandle;
    m_transId = transId;
    m_taskId = std::to_string(bsaHandle);
    m_lastOpRet = MP_SUCCESS;
    m_startTime = CMpTime::GetTimeSec();
    m_pageInfo.limit = BSA_QUERY_LIMIT;
    m_pageInfo.offset = 0;
}

BsaTransaction::~BsaTransaction()
{
    m_createList.clear();
    m_queryList.clear();
    m_delList.clear();
}

BsaTransStatus BsaTransaction::GetCurStatus()
{
    return m_stateMachine.m_curStatus;
}

mp_long BsaTransaction::GetSessionId()
{
    return m_bsaHandle;
}

mp_long BsaTransaction::GetTransId()
{
    return m_transId;
}

mp_uint64 BsaTransaction::GetStartTime()
{
    return m_startTime;
}

mp_string BsaTransaction::GetTaskId()
{
    return m_taskId;
}

std::vector<BsaObjInfo>& BsaTransaction::GetQueryList()
{
    return m_queryList;
}

mp_void BsaTransaction::InitQuery(const BsaObjInfo &cond)
{
    m_pageInfo.limit = BSA_QUERY_LIMIT;
    m_pageInfo.offset = 0;
    m_queryCond = cond;
    m_queryList.clear();
}

const BsaQueryPageInfo &BsaTransaction::GetPageInfo() const
{
    return m_pageInfo;
}

const BsaObjInfo &BsaTransaction::GetQueryCond() const
{
    return m_queryCond;
}

mp_void BsaTransaction::UpdatePageInfo(mp_uint32 count)
{
    if (count > m_pageInfo.limit) {
        ERRLOG("basHandle=%lld,queryed obj count=%u more than limit=%u.", m_bsaHandle, count, m_pageInfo.limit);
        // 本次查询返回的记录数超出limit限制，DME异常，终止分页查询，否则可能因DME错误导致死循环
        m_pageInfo.offset = 0;
    } else if (count < m_pageInfo.limit) {
        // 本次查询返回的记录数少于limit，说明已经全部查询完毕，终止分页查询
        m_pageInfo.offset = 0;
    } else {
        m_pageInfo.offset += count;
    }
}

mp_bool BsaTransaction::NeedQueryNextPage()
{
    return (m_pageInfo.offset > 0);
}

mp_bool BsaTransaction::IsLastOpSucc()
{
    return (m_lastOpRet == MP_SUCCESS);
}

mp_void BsaTransaction::SetLastOpRet(mp_int32 ret)
{
    m_lastOpRet = ret;
}

mp_void BsaTransaction::AddCreatList(const BsaObjInfo &obj)
{
    m_createingCopyId = obj.copyId;
    m_createList[obj.copyId] = obj;
    DBGLOG("CopyId=%llu.", obj.copyId);
}

mp_void BsaTransaction::AddDelList(const BsaObjInfo &obj)
{
    m_delList.push_back(obj);
}

mp_bool BsaTransaction::UpdateEstimatedSize(mp_uint64 estimatedSize)
{
    if (m_createList.count(m_createingCopyId) == 0) {
        ERRLOG("CopyId=%llu not found.", m_createingCopyId);
        return MP_FALSE;
    }
    m_createList[m_createingCopyId].estimatedSize = estimatedSize;
    DBGLOG("CopyId=%llu,estimatedSize=%llu.", m_createingCopyId, estimatedSize);
    return MP_TRUE;
}

mp_int32 BsaTransaction::BeginTxn()
{
    return InitStateMachine();
}

mp_bool BsaTransaction::TxnCanCommit()
{
    BsaTransStatus curStatus = m_stateMachine.m_curStatus;

    // 纯查询事务任意状态下都可以提交
    if ((m_createList.size() + m_delList.size()) == 0) {
        return MP_TRUE;
    }

    // 纯删除事务在BSADeleteObject后可以提交
    if (m_delList.size() != 0 && m_createList.size() == 0) {
        return (curStatus == BSA_TRANS_DEL_OBJ);
    }

    // 删除+创建事务可以在删除后或者创建后提交
    if (m_delList.size() != 0 && m_createList.size() != 0) {
        return (curStatus == BSA_TRANS_DEL_OBJ || curStatus == BSA_TRANS_END_DATA);
    }

    // 创建和读取对象数据事务正常结束的最后一条消息是EndData
    return (curStatus == BSA_TRANS_END_DATA);
}

mp_int32 BsaTransaction::EndTxn(BSA_Vote vote)
{
    m_queryList.clear();

    if (vote == BSA_Vote_COMMIT && !TxnCanCommit()) { // 事务非正常结束无法提交
        ERRLOG("trans cannot commit!bsaHandle=%lld.createList.size=%llu,delList.size=%llu",
            m_bsaHandle, m_createList.size(), m_delList.size());
        return BsaSessionManager::GetInstance().SetLastErr(BSA_RC_INVALID_CALL_SEQUENCE);
    }

    INFOLOG("bsaHandle=%lld,transId=%lld,createList.size=%lu,delList.size=%lu",
        m_bsaHandle, m_transId, m_createList.size(), m_delList.size());

    if (vote == BSA_Vote_ABORT || m_createList.size() == 0) {
        return BSA_RC_SUCCESS;
    }

    if (BsaObjManager::GetInstance().SaveObjects(m_createList, m_bsaHandle) != MP_SUCCESS) {
        ERRLOG("Save objects failed!bsaHandle=%lld.createList.size=%d", m_bsaHandle, m_createList.size());
        return BsaSessionManager::GetInstance().SetLastErr(BSA_RC_ABORT_SYSTEM_ERROR);
    }

    INFOLOG("Save objects success!bsaHandle=%lld,transId=%lld.", m_bsaHandle, m_transId);
    return BSA_RC_SUCCESS;
}

mp_int32 BsaTransaction::CreateObj(BsaObjInfo &obj)
{
    return (m_stateMachine.m_base.get() ? m_stateMachine.m_base->CreateObj(obj) : RET_BASE_NULL);
}

mp_int32 BsaTransaction::SendData()
{
    return (m_stateMachine.m_base.get() ? m_stateMachine.m_base->SendData() : RET_BASE_NULL);
}

mp_int32 BsaTransaction::EndData()
{
    return (m_stateMachine.m_base.get() ? m_stateMachine.m_base->EndData() : RET_BASE_NULL);
}

mp_int32 BsaTransaction::DelObj(mp_uint64 copyId)
{
    return (m_stateMachine.m_base.get() ? m_stateMachine.m_base->DelObj(copyId) : RET_BASE_NULL);
}

mp_int32 BsaTransaction::QueryObj(const BsaObjInfo &queryCond, BsaObjInfo &queryResult)
{
    return (m_stateMachine.m_base.get() ? m_stateMachine.m_base->QueryObj(queryCond, queryResult) : RET_BASE_NULL);
}

mp_int32 BsaTransaction::GetObj()
{
    return (m_stateMachine.m_base.get() ? m_stateMachine.m_base->GetObj() : RET_BASE_NULL);
}

mp_int32 BsaTransaction::GetNextObj(BsaObjInfo &queryResult)
{
    return (m_stateMachine.m_base.get() ? m_stateMachine.m_base->GetNextObj(queryResult) : RET_BASE_NULL);
}

mp_int32 BsaTransaction::GetData()
{
    return (m_stateMachine.m_base.get() ? m_stateMachine.m_base->GetData() : RET_BASE_NULL);
}