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
#ifndef _AGENT_MESSAGE_HANDLER_H_
#define _AGENT_MESSAGE_HANDLER_H_

#include <vector>
#include "common/Types.h"
#include "common/Log.h"
#include "common/CMpThread.h"
#include "message/tcp/CDppMessage.h"
#include "pluginfx/PluginManager.h"

static const mp_int32 MAX_QUEUE_MESSAGE_NUM = 1048576;  // 队列中最多的消息，默认1M

class MessageHandler {
public:
    static MessageHandler& GetInstance();
    virtual ~MessageHandler();

    mp_int32 PushReqMsg(message_pair_t& msgPair);
    mp_int32 PopReqMsg(message_pair_t& msgPair);
    mp_int32 GetFrontReqMsg(message_pair_t& msgPair);

    mp_int32 PushRspMsg(message_pair_t& msgPair);
    mp_int32 PopRspMsg(message_pair_t& msgPair);
    mp_int32 GetFrontRspMsg(message_pair_t& msgPair);

private:
    std::vector<message_pair_t> msgReqList;     // 请求队列
    std::vector<message_pair_t> msgRspList;     // 响应队列
    static MessageHandler singleInst;           // 单例对象
    thread_lock_t lockReqMutex;                 // 请求队列访问互斥锁
    thread_lock_t lockRspMutex;                 // 回复队列访问互斥锁

    // 私有构造函数
    MessageHandler();
};
#endif
