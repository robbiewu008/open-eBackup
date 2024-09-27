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
#ifndef HCS_SERVER_DETAIL_H
#define HCS_SERVER_DETAIL_H

#include <string>
#include <vector>
#include <json/json.h>
#include <common/JsonHelper.h>
#include "protect_engines/hcs/common/HcsMacros.h"

namespace HcsPlugin {
/**
 *  快捷链接信息
 */
struct Link {
    std::string m_href;     // 对应快捷链接
    std::string m_rel;     // 快捷链接标记名称，枚举值：self, bookmark, alternate

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_href, href)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_rel, rel)
    END_SERIAL_MEMEBER
};

/**
 *  云服务器安全组信息
 */
struct ServerSecurityGroup {
    std::string m_name;     // 安全组名称

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    END_SERIAL_MEMEBER
};

/**
 *  云服务器网络配置信息
 */
struct NetWork {
    std::string m_osEXTIPSMACmacAddr;     // MAC地址
    std::string m_osEXTIPStype;     // 分配IP地址方式
    std::string m_addr;     // IP地址信息
    int32_t m_version;     // IP地址类型，值为4或6

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osEXTIPSMACmacAddr, OS-EXT-IPS-MAC:mac_addr)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osEXTIPStype, OS-EXT-IPS:type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_addr, addr)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_version, version)
    END_SERIAL_MEMEBER
};

/**
 *  云服务器镜像信息
 */
struct ServerImage {
    std::string m_uuid;     // 镜像ID
    std::vector<Link> m_links;     // 镜像相关标记快捷链接信息

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_uuid, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_links, links)
    END_SERIAL_MEMEBER
};

/**
 *  云服务器类型信息
 */
struct ServerFlavor {
    std::string m_uuid;     // 云服务器类型ID
    std::vector<Link> m_links;     // 云服务器类型相关标记快捷链接信息

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_uuid, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_links, links)
    END_SERIAL_MEMEBER
};

/**
 *  云服务器卷信息
 */
struct ServerVolume {
    std::string m_uuid;     // 云硬盘ID

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_uuid, id)
    END_SERIAL_MEMEBER
};
}

#endif