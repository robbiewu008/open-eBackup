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
#include "LoginHost.h"
#include "volume_handlers/oceanstor/DiskScannerHandler.h"

namespace {
    static const int32_t MAX_ERROR_MSG_LEN = 256;
} /* namespace */

VIRT_PLUGIN_NAMESPACE_BEGIN

int32_t LoginHost::RunCmd(const SshParam &input, std::string &result)
{
    // 建立socket连接
    int32_t sock;
    if (CreateSocketConnect(input, sock) != SUCCESS) {
        ERRLOG("Create socket connect failed.");
        return FAILED;
    }
    // 创建SSH会话
    libssh2_init(0);
    LIBSSH2_SESSION *session = libssh2_session_init();
    if (session == NULL) {
        ERRLOG("Failed to init libssh2 session.");
        return FAILED;
    }
    libssh2_session_set_blocking(session, 1);
    // 建立握手
    int32_t rc = libssh2_session_handshake(session, sock);
    if (rc != 0) {
        ERRLOG("Handshake failed, %d", rc);
        CleanResource(NULL, session);
        return  FAILED;
    }
    rc = libssh2_userauth_password(session, input.mUserName.c_str(), input.mPassWord.c_str());
    if (rc != 0) {
        ERRLOG("Authentication failed, %d", rc);
        CleanResource(NULL, session);
        return  FAILED;
    }
    // 执行远程命令
    LIBSSH2_CHANNEL *channel = libssh2_channel_open_session(session);
    if (channel == NULL) {
        ERRLOG("Failed to open a channel.");
        CleanResource(channel, session);
        return  FAILED;
    }
    rc = libssh2_channel_request_pty(channel, "xterm");
    if (rc != 0) {
        ERRLOG("Failed to request pty on remote host, %d.", rc);
        CleanResource(channel, session);
        return  FAILED;
    }
    rc = libssh2_channel_exec(channel, input.mCommand.c_str());
    if (rc != 0) {
        ERRLOG("Failed to execute commond on remote host, %d.", rc);
        CleanResource(channel, session);
        return  FAILED;
    }
    ReadResultFromChannel(channel, result);
    DBGLOG("Read result: %s.", result.c_str());
    CleanResource(channel, session);
    return SUCCESS;
}

void LoginHost::ReadResultFromChannel(LIBSSH2_CHANNEL *channel, std::string &result)
{
    int32_t maxSize = 1024 * 24;
    char bufer[maxSize];
    int32_t nbytes;
    do {
        nbytes = libssh2_channel_read(channel, bufer, maxSize - 1);
        if (nbytes <= 0) {
            INFOLOG("Read result from channel failed, ret: %d.", nbytes);
            return;
        }
        bufer[nbytes] = '\0';
        std::string tmp(bufer, bufer + nbytes);
        result += tmp;
        if (nbytes < maxSize - 1) {
            INFOLOG("Read all result.");
            return;
        }
    } while (nbytes > 0);
}

int32_t LoginHost::InitSession(LIBSSH2_SESSION *session, const int32_t &sock)
{
    // 创建SSH会话
    int32_t rc;
    session = libssh2_session_init();
    if (session == NULL) {
        ERRLOG("Failed to init libssh2 session.");
        return FAILED;
    }
    // 设置会话阻塞模式
    libssh2_session_set_blocking(session, 1);
    // 建立握手
    rc = libssh2_session_handshake(session, sock);
    if (rc != 0) {
        ERRLOG("Session handshake failed, %d", rc);
        return FAILED;
    }
    INFOLOG("Init session success");
    return SUCCESS;
}

void LoginHost::CleanResource(LIBSSH2_CHANNEL *channel, LIBSSH2_SESSION *session)
{
    libssh2_channel_close(channel);
    libssh2_channel_free(channel);
    libssh2_session_disconnect(session, "Goodbye");
    libssh2_session_free(session);
    libssh2_exit();
}

int32_t LoginHost::CreateSocketConnect(const SshParam &input, int32_t &sock)
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        int32_t iErr = GetOSError();
        char szErr[MAX_ERROR_MSG_LEN] = {0};
        INFOLOG("Invoke socket failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return FAILED;
    }
    int32_t option = 1;
    int32_t ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(int32_t));
    if (ret == -1) {
        int32_t iErr = GetOSError();
        char szErr[MAX_ERROR_MSG_LEN] = {0};
        INFOLOG("Invoke setsockopt failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return FAILED;
    }
    DiskScannerHandler::GetInstance()->AddIscsiLogicIpRoutePolicy(input.mIp);
    struct sockaddr_in localAddr;
    memset_s(&localAddr, sizeof(localAddr), 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr(input.mIp.c_str());
    localAddr.sin_port = htons(input.mPort);
    if (connect(sock, (struct sockaddr*)(&localAddr), sizeof(struct sockaddr_in)) != 0) {
        int32_t iErr = GetOSError();
        char szErr[256] = {0};
        INFOLOG("Connect failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return FAILED;  // tcp connection error
    }
    if (sock < 0) {
        ERRLOG("Failed to connect to the remote host, ");
        return FAILED;
    }
    INFOLOG("Create socket connect with %s : %d success", input.mIp.c_str(), input.mPort);
    return SUCCESS;
}

int32_t LoginHost::GetOSError()
{
    return errno;
}

char* LoginHost::GetOSStrErr(int32_t err, char buf[], std::size_t buf_len)
{
    int32_t iRet = strncpy_s(buf, buf_len, strerror(err), strlen(strerror(err)));
    if (iRet != 0) {
        return nullptr;
    }
    if (buf_len >= 1) {
        buf[buf_len - 1] = 0;
    }
    return buf;
}
VIRT_PLUGIN_NAMESPACE_END