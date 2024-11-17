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
#ifndef DATA_PATH_PROCESS_H
#define DATA_PATH_PROCESS_H

#include "common/Defines.h"
#include "common/Types.h"
#include "dataprocess/datamessage/DataMessage.h"
#include "dataprocess/datapath/DataPath.h"

// 定义命令枚举变量 -- 可以取代CDppMessage.h中相关命令字的定义
typedef enum tag_cdp_command {
    CMD_PROTECT_VOL = 0,
    CMD_ADD_VOL,
    CMD_DEL_VOL,
    CMD_MOD_VOL,
    CMD_VOL_READY,
    CMD_PAUSE,
    CMD_RESUME,
    CMD_COUNT
} cdp_cmd;

class CdpDataPath : public DataPath {
public:
    CdpDataPath(const mp_string& dpParam);
    ~CdpDataPath();

    // Handle Message received in the base class
    mp_int32 ExtCmdProcess(CDppMessage &message);
    static mp_int32 HandleProtect(Json::Value bodyMsg);
    static mp_int32 HandleAddVol(Json::Value bodyMsg);
    static mp_int32 HandleDelVol(Json::Value bodyMsg);
    static mp_int32 HandleModVol(Json::Value bodyMsg);
    static mp_int32 HandleVolReady(Json::Value bodyMsg);
    static mp_int32 HandlePause(Json::Value bodyMsg);
    static mp_int32 HandleResume(Json::Value bodyMsg);
    mp_void HandleClose();

private:
    mp_int32 ParseCdpCommand(mp_int32 cmd, const Json::Value& bodyMsg);
};

#endif