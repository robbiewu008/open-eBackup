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
#ifndef AGENT_BASIC_MESSAGE_PROCESS_H
#define AGENT_BASIC_MESSAGE_PROCESS_H

#include "common/Defines.h"
#include "common/Types.h"

// 消息体的基础类
class CMessage {
public:
    CMessage()
    {
        m_iType = 0;
    }
    virtual ~CMessage()
    {}
    mp_int32 GetTypeID()
    {
        return m_iType;
    }
protected:
    mp_int32 m_iType;
};

class CBasicReqMsg : public virtual  CMessage {};

class CBasicRspMsg : public virtual  CMessage {
public:
    mp_int64 GetRetCode()
    {
        return m_lRetCode;
    }
    mp_void SetRetCode(mp_int64 lRet)
    {
        m_lRetCode = lRet;
    }

protected:
    mp_int64 m_lRetCode;  // 返回码
};

// 消息对，消息队列存储的数据类型
typedef struct tag_message_pair_t {
    CBasicReqMsg* pReqMsg;
    CBasicRspMsg* pRspMsg;
    tag_message_pair_t()
    {
        pReqMsg = NULL;
        pRspMsg = NULL;
    }
    tag_message_pair_t(CBasicReqMsg& pReq, CBasicRspMsg& pRsp)
    {
        pReqMsg = &pReq;
        pRspMsg = &pRsp;
    }
} message_pair_t;

#endif
