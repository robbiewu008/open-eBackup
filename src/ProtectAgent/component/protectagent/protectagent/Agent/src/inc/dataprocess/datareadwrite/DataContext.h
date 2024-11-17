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
#ifndef __DATA_CONTEXT_H__
#define __DATA_CONTEXT_H__

#include <map>
#include "common/Log.h"
#include "common/Types.h"
#include "common/Defines.h"
#include "common/Types.h"
#include "message/tcp/CSocket.h"
class DataContext {
public:
    mp_void SetSockFd(mp_socket sFd);
    mp_socket GetSockFd();
    mp_int32 GetDiskFdByName(const mp_string &diskName);
    mp_int32 SetDiskFdByName(const mp_string &diskName, mp_int32 diskFd);

private:
    mp_socket sockFd;
    std::map<mp_string, mp_int32> mapOfDisks;
};
#endif