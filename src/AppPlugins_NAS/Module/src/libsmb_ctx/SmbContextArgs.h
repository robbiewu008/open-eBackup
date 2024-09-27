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
#ifndef LIBSMB_CONTEXT_ARGS_H
#define LIBSMB_CONTEXT_ARGS_H

#include "common/CleanMemPwd.h"

namespace Module {

enum class SmbVersion {
    VERSION0311 = 0x0311,
    VERSION0302 = 0x0302,
    VERSION0300 = 0x0300,
    VERSION0210 = 0x0210,
    VERSION0202 = 0x0202
};

enum class SmbAuthType {
    NTLMSSP = 1,
    KRB5 = 2
};

struct SmbContextArgs {
    /* domain: 域 */
    std::string domain;
    /* server: 服务端IP，支持IPv4/IPv6 */
    std::string server;
    /* share: 共享名称，用来访问CIFS共享 */
    std::string share;
    /* user: 访问共享的用户，CIFS共享可以针对不同的用户提供不同的权限 */
    std::string user;
    /* password: 用户密码 */
    std::string password;
    /* krb5CcacheFile: kerberose票据文件 */
    std::string krb5CcacheFile;
    /* krb5ConfigFile: kerberose配置文件 */
    std::string krb5ConfigFile;

    /* encryption: 是否加密连接，仅SMB3.0以上版本支持加密 */
    bool encryption = false;
    /* sign: 是否需要签名 */
    bool sign = false;
    /* timeout: 超时单位：秒，超时未响应的报文被丢弃并返回IO_TIMEOUT异常 */
    int timeout = 60;
    /* authType: 认证类型，支持NTLMSSP和KRB5两种 */
    SmbAuthType authType = SmbAuthType::NTLMSSP;
    /* version: 协议版本，包括3.1.1、3.02、3.0、2.1、2.02 */
    SmbVersion version = SmbVersion::VERSION0300;

    ~SmbContextArgs() { Module::CleanMemoryPwd(password); } // 删除铭感信息
};

}

#endif