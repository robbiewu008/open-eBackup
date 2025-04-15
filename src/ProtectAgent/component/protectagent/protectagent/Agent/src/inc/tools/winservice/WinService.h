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
#ifndef WIN_SERVIICE_HEADER_H
#define WIN_SERVIICE_HEADER_H

#ifdef WIN32
#include "common/Types.h"

// Windows下句柄类单例
class WinService {
public:
    static WinService& GetInstace();
    SERVICE_STATUS_HANDLE& GetServiceHandle();
    mp_void InitServiceHandle();

private:
    WinService();
    ~WinService() {}
    SERVICE_STATUS_HANDLE hServiceStatus;
};
#endif

#endif
