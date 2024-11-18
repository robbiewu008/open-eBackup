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
#ifndef __AGENT_TSF_MESSAGE_IMPL_H__
#define __AGENT_TSF_MESSAGE_IMPL_H__

#include "common/Types.h"

static const mp_uint32 MSG_PREFIX = 0x72634552;

struct DppMessage {
    mp_string uiIpAddr;     // peer ip addr
    mp_uint16 uiPort;       // peer ip port
    mp_time creationTime;   // create time

    // DPP message header
    mp_uint32 uiPrefix;    // magic
    mp_uint16 uiCmd;       // cmd_type
    mp_uint16 uiFlag;      // 'flag' defined in DPP msg header
    mp_uint64 uiOrgSeqNo;  // sequence_num
    mp_uint64 uiSize;      // body_len

    // DPP message body
    mp_char* body;
};

// PART1: message header, prefix (4), cmd (2), flag (2), orgSegNo (8), size (4)
static const mp_uint32 MSG_SIZE_PART1 = 20;

// PART2: body (size) (we don't have suffix in DPP)
#define MSG_SIZE_PART2(m) ((m)->uiSize)

#endif  // __AGENT_TSF_MESSAGE_IMPL_H__
