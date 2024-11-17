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
#include "message/tcp/MessageHandler.h"
#include "common/Log.h"
#include "message/Message.h"

using namespace std;
MessageHandler MessageHandler::singleInst;

MessageHandler& MessageHandler::GetInstance()
{
    return singleInst;
}

MessageHandler::~MessageHandler()
{
    {
        CThreadAutoLock lock(&lockReqMutex);
        while (!msgReqList.empty()) {
            message_pair_t msgPair = msgReqList.front();
            msgReqList.erase(msgReqList.begin());
            delete msgPair.pReqMsg;
            delete msgPair.pRspMsg;
        }
    }

    {
        CThreadAutoLock lock(&lockRspMutex);
        while (!msgRspList.empty()) {
            message_pair_t msgPair = msgRspList.front();
            msgRspList.erase(msgRspList.begin());
            delete msgPair.pReqMsg;
            delete msgPair.pRspMsg;
        }
    }
    CMpThread::DestroyLock(&lockReqMutex);
    CMpThread::DestroyLock(&lockRspMutex);
}

MessageHandler::MessageHandler()
{
    CMpThread::InitLock(&lockReqMutex);
    CMpThread::InitLock(&lockRspMutex);
}

// 如果添加消息过多，则需要判断最大消息数量，如果消息过多需要返回异常
mp_int32 MessageHandler::PushReqMsg(message_pair_t& msgPair)
{
    CThreadAutoLock lock(&lockReqMutex);
    if (msgReqList.size() >= MAX_QUEUE_MESSAGE_NUM) {
        COMMLOG(OS_LOG_ERROR, "msgReqList have exceeded max number");
        return MP_FAILED;
    }
    msgReqList.push_back(msgPair);
    COMMLOG(OS_LOG_DEBUG, "PushReqMsg sucees, smsgReqList size %d.", msgReqList.size());
    return MP_SUCCESS;
}

mp_int32 MessageHandler::PopReqMsg(message_pair_t& msgPair)
{
    CThreadAutoLock lock(&lockReqMutex);
    if (!msgReqList.empty()) {
        msgPair = msgReqList.front();
        msgReqList.erase(msgReqList.begin());
        COMMLOG(OS_LOG_DEBUG, "PopReqMsg sucees, msgReqList size %d.", msgReqList.size());
        return MP_SUCCESS;
    } else {
        return MP_FAILED;
    }
}


// 获取队列头中的消息
mp_int32 MessageHandler::GetFrontReqMsg(message_pair_t& msgPair)
{
    CThreadAutoLock lock(&lockReqMutex);
    if (!msgReqList.empty()) {
        msgPair = msgReqList.front();
        COMMLOG(OS_LOG_DEBUG, "GetFrontReqMsg sucess,msgReqList size %d.", msgReqList.size());
        return MP_SUCCESS;
    } else {
        return MP_FAILED;
    }
}

// 如果添加消息过多，则需要判断最大消息数量，如果消息过多需要返回异常
mp_int32 MessageHandler::PushRspMsg(message_pair_t& msgPair)
{
    CThreadAutoLock lock(&lockRspMutex);
    if (msgRspList.size() >= MAX_QUEUE_MESSAGE_NUM) {
        COMMLOG(OS_LOG_ERROR, "msgRspList have exceeded max number");
        return MP_FAILED;
    }
    msgRspList.push_back(msgPair);
    COMMLOG(OS_LOG_DEBUG, "msgRspList size %d.", msgRspList.size());
    return MP_SUCCESS;
}

mp_int32 MessageHandler::PopRspMsg(message_pair_t& msgPair)
{
    CThreadAutoLock lock(&lockRspMutex);
    if (!msgRspList.empty()) {
        msgPair = msgRspList.front();
        msgRspList.erase(msgRspList.begin());
        COMMLOG(OS_LOG_DEBUG, "Remain msgRspList size %d.", msgRspList.size());
        return MP_SUCCESS;
    } else {
        return MP_FAILED;
    }
}

// 获取队列头中的消息
mp_int32 MessageHandler::GetFrontRspMsg(message_pair_t& msgPair)
{
    CThreadAutoLock lock(&lockRspMutex);
    if (!msgRspList.empty()) {
        msgPair = msgRspList.front();
        COMMLOG(OS_LOG_DEBUG, "GetFrontRspMsg sucess, Remain msgRspList size %d.", msgRspList.size());
        return MP_SUCCESS;
    } else {
        return MP_FAILED;
    }
}