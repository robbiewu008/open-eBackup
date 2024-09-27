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
#ifndef LOGIN_HOST_H
#define LOGIN_HOST_H

#include <libssh2.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include "securec.h"
#include "log/Log.h"
#include "common/Macros.h"
#include "common/Constants.h"
#include "volume_handlers/fusionstorage/FusionStorageStructs.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

class SshParam {
public:
    SshParam() {}
    explicit SshParam(VBSNodeInfo vbsInfo) : mIp(vbsInfo.mNodeIp), mPort(vbsInfo.mPort),
        mUserName(vbsInfo.mUserName), mPassWord(vbsInfo.mPassWord)
    {}
    ~SshParam() {}

    std::string mIp;
    int32_t mPort;
    std::string mUserName;
    std::string mPassWord;
    std::string mCommand;
};

class LoginHost {
public:
    LoginHost() {}
    ~LoginHost() {}

    int32_t RunCmd(const SshParam &input, std::string &result);
    int32_t CreateSocketConnect(const SshParam &input, int32_t &sock);

    int32_t GetOSError();
    int32_t InitSession(LIBSSH2_SESSION *session, const int32_t &sock);
    void CleanResource(LIBSSH2_CHANNEL *channel, LIBSSH2_SESSION *session);
    char* GetOSStrErr(int32_t err, char buf[], std::size_t buf_len);
    void ReadResultFromChannel(LIBSSH2_CHANNEL *channel, std::string &result);
};
VIRT_PLUGIN_NAMESPACE_END
#endif