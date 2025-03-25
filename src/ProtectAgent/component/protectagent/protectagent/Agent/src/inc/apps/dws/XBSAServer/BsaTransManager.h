#ifndef _BSA_TRANS_MANAGER_H_
#define _BSA_TRANS_MANAGER_H_

#include <vector>
#include <mutex>
#include "common/Types.h"
#include "xbsa/xbsa.h"
#include "apps/dws/XBSAServer/xbsa_types.h"
#include "apps/dws/XBSAServer/BsaTransState.h"
#include "apps/dws/XBSAServer/BsaObjManager.h"

class BsaTransStateMachine {
public:
    BsaTransStateMachine() { };
    ~BsaTransStateMachine() { };

    BsaTransStatus m_curStatus;
    std::mutex m_statusMutex;
    std::unique_ptr<BsaTransStateBase> m_base;
};

class BsaTransaction {
public:
    BsaTransaction(mp_long bsaHandle, mp_long transId);
    ~BsaTransaction();

    mp_int32 CreateObj(BsaObjInfo &obj);
    mp_int32 SendData();
    mp_int32 EndData();
    mp_int32 DelObj(mp_uint64 copyId);
    mp_int32 QueryObj(const BsaObjInfo &queryCond, BsaObjInfo &queryResult);
    mp_int32 GetObj();
    mp_int32 GetNextObj(BsaObjInfo &queryResult);
    mp_int32 GetData();

    mp_int32 BeginTxn();
    mp_int32 EndTxn(BSA_Vote vote);

    mp_int32 ChangeState(BsaTransStatus status);
    BsaTransStatus GetCurStatus();
    mp_long GetSessionId();
    mp_long GetTransId();
    mp_uint64 GetStartTime();
    mp_string GetTaskId();
    std::vector<BsaObjInfo>& GetQueryList();

    mp_void InitQuery(const BsaObjInfo &cond);
    const BsaQueryPageInfo &GetPageInfo() const;
    const BsaObjInfo &GetQueryCond() const;
    mp_void UpdatePageInfo(mp_uint32 count);
    mp_bool NeedQueryNextPage();

    mp_bool IsLastOpSucc();
    mp_void SetLastOpRet(mp_int32 ret);
    mp_void AddCreatList(const BsaObjInfo &obj);
    mp_void AddDelList(const BsaObjInfo &obj);
    mp_bool UpdateEstimatedSize(mp_uint64 estimatedSize);

private:
    mp_int32 InitStateMachine();
    mp_bool TxnCanCommit();

    mp_long m_bsaHandle;
    mp_long m_transId;
    mp_string m_taskId;
    BsaTransStateMachine m_stateMachine;
    mp_uint64 m_startTime; // 事务启动时间（相对时间，从系统启动开始计时，单位为秒）
    mp_int32 m_lastOpRet; // 上一次操作的结果，当前只针对调用DPP接口出错记录了失败结果
    BsaQueryPageInfo m_pageInfo; // 本次查询页信息
    BsaObjInfo m_queryCond; // 本次查询条件
    mp_uint64 m_createingCopyId{0}; // 正在创建的对象的copyId
    std::map<mp_uint64, BsaObjInfo> m_createList; // 正在创建的对象列表（尚未提交）
    std::vector<BsaObjInfo> m_queryList;  // 本次查询结果
    std::vector<BsaObjInfo> m_delList;    // 正在删除的对象列表（尚未提交）
};

#endif // _BSA_TRANS_MANAGER_H_