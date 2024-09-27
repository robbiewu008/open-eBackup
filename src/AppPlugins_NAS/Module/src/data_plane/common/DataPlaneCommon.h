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
#ifndef DP_COMMON_H
#define DP_COMMON_H

#include <iostream>

namespace Module {

struct AuthInfo {
    std::string caFilePath;
    std::string certFilePath;
    std::string keyFilePath;
    std::string kmcStoreFilePath;
    std::string kmcBackupStoreFilePath;
    std::string encryPwdFilePath;
    std::string encryPwd;
};

typedef struct AuthInfo AuthObjInfo;

struct ServerInfo {
    std::string ip;
    std::uint16_t port;
    AuthObjInfo authInfo;
};

typedef struct ServerInfo ServerObjInfo;

}
#endif // DP_COMMON_H