/* *
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * Description: XBSA transaction state machine definition.
 * Create: 2021-05-18
 * Author: wuchuan wwx563312
 */
#ifndef _BSA_TRANS_STATE_H_
#define _BSA_TRANS_STATE_H_

#include "common/Types.h"
#include "xbsa/xbsa.h"
#include "apps/dws/XBSAServer/xbsa_types.h"
#include "apps/dws/XBSAServer/BsaObjManager.h"

class BsaTransaction;
typedef enum tagBsaTransStatus {
    BSA_TRANS_IDLE,
    BSA_TRANS_CREATE_OBJ,
    BSA_TRANS_SEND_DATA,
    BSA_TRANS_DEL_OBJ,
    BSA_TRANS_QUERY_OBJ,
    BSA_TRANS_GET_OBJ,
    BSA_TRANS_GET_NEXT_OBJ,
    BSA_TRANS_GET_DATA,
    BSA_TRANS_END_DATA,
    BSA_TRANS_BUTT
} BsaTransStatus;

// 状态机基类，默认所有接口都不支持，在各状态子类中实现该子类对应状态下需要支持的接口
class BsaTransStateBase {
public:
    BsaTransStateBase(BsaTransaction *obj) : m_trans(obj) {};
    ~BsaTransStateBase() {};

    virtual mp_int32 CreateObj(BsaObjInfo &obj)
    {
        return StateErr();
    }
    virtual mp_int32 SendData()
    {
        return StateErr();
    }
    virtual mp_int32 EndData()
    {
        return StateErr();
    }
    virtual mp_int32 DelObj(mp_uint64 copyId)
    {
        return StateErr();
    }
    virtual mp_int32 QueryObj(const BsaObjInfo &queryCond, BsaObjInfo &queryResult)
    {
        return StateErr();
    }
    virtual mp_int32 GetObj()
    {
        return StateErr();
    }
    virtual mp_int32 GetNextObj(BsaObjInfo &queryResult)
    {
        return StateErr();
    }
    virtual mp_int32 GetData()
    {
        return StateErr();
    }

    // doXxx是真正处理XBSA请求的函数
    mp_int32 DoCreateObj(BsaObjInfo &obj);
    mp_int32 DoSendData();
    mp_int32 DoEndData();
    mp_int32 DoDelObj(mp_uint64 copyId);
    mp_int32 DoQueryObj(const BsaObjInfo &queryCond, BsaObjInfo &queryResult);
    mp_int32 DoGetObj();
    mp_int32 DoGetNextObj(BsaObjInfo &queryResult);
    mp_int32 DoGetData();

    mp_int32 ChangeState(BsaTransStatus status);

private:
    mp_int32 StateErr(); // 公共的错误处理函数
    mp_void GetNextQueryObj(BsaObjInfo &queryResult);
    BsaTransaction *m_trans;
};

class BsaTransStateIdle : public BsaTransStateBase {
public:
    BsaTransStateIdle(BsaTransaction *obj) : BsaTransStateBase(obj) {};
    ~BsaTransStateIdle() {};
    mp_int32 CreateObj(BsaObjInfo &obj) override;
    mp_int32 QueryObj(const BsaObjInfo &queryCond, BsaObjInfo &queryResult) override;
    mp_int32 GetObj() override;
    mp_int32 DelObj(mp_uint64 copyId) override;
};

class BsaTransStateCreateObj : public BsaTransStateBase {
public:
    BsaTransStateCreateObj(BsaTransaction *obj) : BsaTransStateBase(obj) {};
    ~BsaTransStateCreateObj() {};
    mp_int32 SendData() override;
    mp_int32 EndData() override;
    mp_int32 QueryObj(const BsaObjInfo &queryCond, BsaObjInfo &queryResult) override;
};

class BsaTransStateSendData : public BsaTransStateBase {
public:
    BsaTransStateSendData(BsaTransaction *obj) : BsaTransStateBase(obj) {};
    ~BsaTransStateSendData() {};
    mp_int32 SendData() override;
    mp_int32 EndData() override;
};

class BsaTransStateDelObj : public BsaTransStateBase {
public:
    BsaTransStateDelObj(BsaTransaction *obj) : BsaTransStateBase(obj) {};
    ~BsaTransStateDelObj() {};
    mp_int32 DelObj(mp_uint64 copyId) override;
    mp_int32 EndData() override;
    mp_int32 CreateObj(BsaObjInfo &obj) override;
};

class BsaTransStateQueryObj : public BsaTransStateBase {
public:
    BsaTransStateQueryObj(BsaTransaction *obj) : BsaTransStateBase(obj) {};
    ~BsaTransStateQueryObj() {};
    mp_int32 GetObj() override;
    mp_int32 QueryObj(const BsaObjInfo &queryCond, BsaObjInfo &queryResult) override;
    mp_int32 GetNextObj(BsaObjInfo &queryResult) override;
    mp_int32 DelObj(mp_uint64 copyId) override; // 适配roach_client 8.1.0在query后直接delete
    mp_int32 EndData() override;
};

class BsaTransStateGetObj : public BsaTransStateBase {
public:
    BsaTransStateGetObj(BsaTransaction *obj) : BsaTransStateBase(obj) {};
    ~BsaTransStateGetObj() {};
    mp_int32 GetObj() override;
    mp_int32 GetData() override;
    mp_int32 GetNextObj(BsaObjInfo &queryResult) override;
    mp_int32 EndData() override;
};

class BsaTransStateGetNextObj : public BsaTransStateBase {
public:
    BsaTransStateGetNextObj(BsaTransaction *obj) : BsaTransStateBase(obj) {};
    ~BsaTransStateGetNextObj() {};
    mp_int32 GetObj() override;
    mp_int32 GetData() override;
    mp_int32 QueryObj(const BsaObjInfo &queryCond, BsaObjInfo &queryResult) override;
    mp_int32 GetNextObj(BsaObjInfo &queryResult) override;
    mp_int32 DelObj(mp_uint64 copyId) override;
    mp_int32 EndData() override;
};

class BsaTransStateGetData : public BsaTransStateBase {
public:
    BsaTransStateGetData(BsaTransaction *obj) : BsaTransStateBase(obj) {};
    ~BsaTransStateGetData() {};
    mp_int32 GetData() override;
    mp_int32 EndData() override;
};

// 调用EndData()后，trans进入idle状态，因此EndData子状态跟Idle子状态完全等同.
class BsaTransStateEndData : public BsaTransStateIdle {
public:
    BsaTransStateEndData(BsaTransaction *obj) : BsaTransStateIdle(obj) {};
    ~BsaTransStateEndData() {};
    mp_int32 GetNextObj(BsaObjInfo &queryResult) override;
};

#endif // _BSA_TRANS_STATE_H_