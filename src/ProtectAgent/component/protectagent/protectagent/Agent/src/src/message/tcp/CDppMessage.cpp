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
#include "message/tcp/CDppMessage.h"
#if defined(AIX) || defined(SOLARIS)
#include <sys/types.h>
#include <netinet/in.h>
#endif
#include "common/Ip.h"
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "common/Types.h"
namespace {
const int OFFET = 3;
const int UINT16SWAP = 2;
const int UINT32SWAP = 4;
const int UINT64SWAP = 8;
const int SWAPSTART = 0;
}


mp_void CDppMessage::ItemEendianSwap()
{
    DEndianSwapl(&dppMessage.uiPrefix);
    DEndianSwaps(&dppMessage.uiCmd);
    DEndianSwaps(&dppMessage.uiFlag);
    DEndianSwapll(&dppMessage.uiOrgSeqNo);
    DEndianSwapll(&dppMessage.uiSize);
}
mp_void CDppMessage::DEndianSwaps(mp_uint16 *pData)
{
    mp_char* p = (mp_char*)(pData);
    EndianSwap(p, SWAPSTART, UINT16SWAP);
}

mp_void CDppMessage::DEndianSwapl(mp_uint32 *pData)
{
    mp_char* p = (mp_char*)(pData);
    EndianSwap(p, SWAPSTART, UINT32SWAP);
}

mp_void CDppMessage::DEndianSwapll(mp_uint64 *pData)
{
    mp_char* p = (mp_char*)(pData);
    EndianSwap(p, SWAPSTART, UINT64SWAP);
}

mp_void CDppMessage::EndianSwap(mp_char *pData, int startIndex, int length)
{
    mp_int32 cnt = length / UINT16SWAP;
    mp_int32 start = startIndex;
    mp_int32 end = startIndex + length - 1;
    mp_char tmp;
    for (mp_int32 i = 0; i < cnt; i++) {
        tmp = pData[start + i];
        pData[start + i] = pData[end - i];
        pData[end - i] = tmp;
    }
}

CDppMessage::CDppMessage()
{
    m_iType = DPPMESSAGE_TYPE;
    dppMessage.body = NULL;
    dppMessage.uiSize = 0;
    mCmd = MANAGE_CMD_NO_INVALID;
    mErrNo = MANAGE_ERRNO_INVALID;
}

CDppMessage::~CDppMessage()
{
    DestroyMsgBody();
}

CDppMessage::CDppMessage(const CDppMessage &msg)
{
    m_iType = DPPMESSAGE_TYPE;
    dppMessage = msg.dppMessage;
    mCmd = msg.mCmd;
    manageBody = msg.manageBody;
    mErrNo = msg.mErrNo;
    mErrDetail = msg.mErrDetail;
    mLastUpTime = msg.mLastUpTime;

    dppMessage.body = NULL;
    if (InitMsgBody() != MP_SUCCESS) {
        return;
    }
    memcpy_s(dppMessage.body, dppMessage.uiSize, msg.dppMessage.body, msg.dppMessage.uiSize);
}

mp_void CDppMessage::InitMsgHead(mp_uint16 cmdNo, mp_uint16 flag, mp_uint64 seqNo)
{
    dppMessage.uiIpAddr = "";
    dppMessage.uiPort = 0;
    dppMessage.uiPrefix = MSG_PREFIX;
    dppMessage.uiCmd = cmdNo;
    dppMessage.uiFlag = flag;
    dppMessage.uiOrgSeqNo = seqNo;
    dppMessage.uiSize = 0;

    DestroyMsgBody();
}

mp_int32 CDppMessage::InitMsgBody()
{
    // check message body size
    if (dppMessage.uiSize > MAX_MESSAGE_SIZE || dppMessage.uiSize == 0) {
        COMMLOG(OS_LOG_ERROR, "invalid message size %llu.", dppMessage.uiSize);
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "message body size %llu.", dppMessage.uiSize);
    // free old message body
    DestroyMsgBody();
    dppMessage.body = (mp_char*)calloc(1, dppMessage.uiSize + 1);
    if (dppMessage.body == NULL) {
        COMMLOG(OS_LOG_ERROR, "new message body failed.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

// 用于回复消息的时候，只设置body体，发送前赋值sessionId后再发送
mp_int32 CDppMessage::ReinitMsgBody()
{
    mp_string JsonStr = manageBody.toStyledString();
    dppMessage.uiSize = JsonStr.size() + 1;

    mp_int32 iRet = InitMsgBody();
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    memcpy_s(dppMessage.body, JsonStr.size(), JsonStr.c_str(), JsonStr.size());
    dppMessage.body[dppMessage.uiSize - 1] = '\0';
    return MP_SUCCESS;
}

mp_void CDppMessage::CloneMsg(CDppMessage& msg)
{
    dppMessage.uiPrefix = MSG_PREFIX;
    dppMessage.uiCmd = msg.GetCmd();
    dppMessage.uiIpAddr = msg.GetIpAddr();
    dppMessage.uiPort = msg.GetPort();
    dppMessage.uiOrgSeqNo = msg.GetOrgSeqNo();
    SetMsgTgt(msg.GetMsgSrc());
    SetMsgSrc(msg.GetMsgTgt());
}

mp_string CDppMessage::GetIpAddr()
{
    return dppMessage.uiIpAddr;
}

mp_uint16 CDppMessage::GetPort()
{
    return dppMessage.uiPort;
}

mp_string CDppMessage::GetIpAddrStr()
{
    return GetIpAddr();
}

mp_bool CDppMessage::IsValidPrefix()
{
#if defined(AIX) || defined(SOLARIS)
    ItemEendianSwap();
#endif
    return (dppMessage.uiPrefix == MSG_PREFIX);
}

mp_uint32 CDppMessage::GetPrefix()
{
    return dppMessage.uiPrefix;
}

mp_uint16 CDppMessage::GetCmd()
{
    return dppMessage.uiCmd;
}

mp_uint16 CDppMessage::GetFlag()
{
    return dppMessage.uiFlag;
}

mp_uint64 CDppMessage::GetOrgSeqNo()
{
    return dppMessage.uiOrgSeqNo;
}

mp_uint32 CDppMessage::GetSize()
{
    return GetSize1() + GetSize2();
}

mp_uint32 CDppMessage::GetSize1()
{
    return MSG_SIZE_PART1;
}

mp_uint32 CDppMessage::GetSize2()
{
    return dppMessage.uiSize;
}

mp_char* CDppMessage::GetBuffer()
{
    return dppMessage.body;
}

mp_char* CDppMessage::GetStart()
{
    return (mp_char*)&dppMessage.uiPrefix;
}

mp_void CDppMessage::SetMsgSrc(MESSAGE_ROLE role)
{
    dppMessage.uiFlag = (dppMessage.uiFlag & 0xFFF8) | role;
}

mp_void CDppMessage::SetMsgTgt(MESSAGE_ROLE role)
{
    dppMessage.uiFlag = (dppMessage.uiFlag & 0xFFC7) | (role << OFFET);
}

MESSAGE_ROLE CDppMessage::GetMsgSrc()
{
    return (MESSAGE_ROLE)(dppMessage.uiFlag & 0x07);
}

MESSAGE_ROLE CDppMessage::GetMsgTgt()
{
    return (MESSAGE_ROLE)((dppMessage.uiFlag & 0x38) >> OFFET);
}

mp_void CDppMessage::SwapSrcTgt()
{
    mp_char src = dppMessage.uiFlag & 0x07;
    mp_char tgt = dppMessage.uiFlag & 0x38;

    dppMessage.uiFlag = (dppMessage.uiFlag & 0xFFF8) | tgt;
    dppMessage.uiFlag = (dppMessage.uiFlag & 0xFFC7) | (src << OFFET);
}

mp_int32 CDppMessage::SetMsgBody(Json::Value& msgBody)
{
    this->manageBody = msgBody;
    return MP_SUCCESS;
}

mp_void CDppMessage::SetLinkInfo(const mp_string& uiIpAddr, const mp_uint16& uiPort)
{
    dppMessage.uiIpAddr = uiIpAddr;
    dppMessage.uiPort = uiPort;
}

mp_void CDppMessage::SetOrgSeqNo(mp_uint64 uiOrgSeqNo)
{
    dppMessage.uiOrgSeqNo = uiOrgSeqNo;
}

mp_uint32 CDppMessage::GetManageCmd()
{
    if (this->mCmd != MANAGE_CMD_NO_INVALID) {
        return this->mCmd;
    }

    mp_int32 iRet = AnalyzeManageMsg();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "AnalyzeManageMsg failed, return %d.", iRet);
        return MANAGE_CMD_NO_INVALID;
    } else {
        return this->mCmd;
    }
}

mp_int32 CDppMessage::GetManageBody(Json::Value& dppBody)
{
    if (!manageBody.empty()) {
        dppBody = this->manageBody;
        return MP_SUCCESS;
    }

    mp_int32 iRet = AnalyzeManageMsg();
    if (iRet == MP_SUCCESS) {
        dppBody = this->manageBody;
    } else {
        COMMLOG(OS_LOG_ERROR, "AnalyzeManageMsg failed, return %d.", iRet);
    }
    return iRet;
}

mp_int32 CDppMessage::GetManageError(mp_int64& errNo, mp_string& errDetail)
{
    if (mErrNo != MANAGE_ERRNO_INVALID) {
        errNo = mErrNo;
        errDetail = mErrDetail;
        return MP_SUCCESS;
    }

    mp_int32 iRet = AnalyzeManageMsg();
    if (iRet == MP_SUCCESS) {
        errNo = mErrNo;
        errDetail = mErrDetail;
    } else {
        COMMLOG(OS_LOG_ERROR, "AnalyzeManageMsg failed, return %d.", iRet);
    }
    return iRet;
}

mp_void CDppMessage::DestroyMsgBody()
{
    if (dppMessage.body != NULL) {
        free(dppMessage.body);
        dppMessage.body = NULL;
        mCmd = MANAGE_CMD_NO_INVALID;
        mErrNo = MANAGE_ERRNO_INVALID;
        if (mErrDetail.empty()) {
            return;
        }
        memset_s(&mErrDetail[0], mErrDetail.length(), 0, mErrDetail.length());
    }
}

mp_int32 CDppMessage::AnalyzeManageMsg()
{
    if (!dppMessage.body || strlen(dppMessage.body) == 0) {
        return MP_SUCCESS;
    }
    try {
        if (dppMessage.body[0] != '{') {
            COMMLOG(OS_LOG_ERROR, "Message body info invalid.");
            return MP_FAILED;
        }

        Json::Reader reader;
        mp_bool bRet = reader.parse(dppMessage.body, this->manageBody);
        if (bRet != MP_TRUE) {
            COMMLOG(OS_LOG_ERROR, "Parse message body info failed");
            return MP_FAILED;
        }
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "JsonData is invalid.");
        return MP_FAILED;
    }

    mp_int32 iRet = CJsonUtils::GetJsonUInt32(this->manageBody, MANAGECMD_KEY_CMDNO, this->mCmd);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get Manage cmd node failed, ret=%d.", iRet);
        return MP_FAILED;
    }

    // send message have no error and error detail
    GET_JSON_INT64_OPTION(this->manageBody, MANAGECMD_KEY_ERRORCODE, this->mErrNo);
    GET_JSON_STRING_OPTION(this->manageBody, MANAGECMD_KEY_ERRORDETAIL, this->mErrDetail);
    return MP_SUCCESS;
}

mp_void CDppMessage::UpdateTime()
{
    CMpTime::Now(mLastUpTime);
}

mp_time CDppMessage::GetUpdateTime()
{
    return mLastUpTime;
}
