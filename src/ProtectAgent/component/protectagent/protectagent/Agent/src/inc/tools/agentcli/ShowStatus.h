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
#ifndef _AGENTCLI_SHOWSTATUS_H_
#define _AGENTCLI_SHOWSTATUS_H_

#include "common/Types.h"

static const mp_string RUNNING_TAG = "RUNNING";
static const mp_string SVN_CONF = "svn";

typedef enum { PROCESS_RDAGENT = 0, PROCESS_NGINX, PROCESS_MONITOR } PROCESS_TYPE;

class ShowStatus {
public:
    static mp_int32 Handle();

private:
    static mp_bool IsStartted(PROCESS_TYPE eType);
    static mp_void ShowSvn();
};

#endif
