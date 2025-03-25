/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file MessageHandler.h
 * @brief  The implemention MessageHandler
 * @version 1.0.0.0
 * @date 2019-11-15
 * @author wangguitao 00510599
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
