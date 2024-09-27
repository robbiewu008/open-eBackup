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
package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Data;

/**
 * Agent健康检查接口返回数据模型
 *
 * @author w00616953
 * @since 2021-11-30
 */
@Data
public class HostDto {
    /**
     * 环境uuid
     */
    private String uuid;

    /**
     * 环境名称
     */
    private String name;

    /**
     * 环境主类型
     */
    private String type;

    /**
     * 环境子类型
     */
    private String subType;

    /**
     * 环境的IP地址或域名
     */
    private String endpoint;

    /**
     * 端口
     */
    private Integer port;

    /**
     * 连接环境的用户名
     */
    private String username;

    /**
     * 连接环境的密码
     */
    private String password;

    /**
     * 环境的操作类型
     */
    private String osType;

    /**
     * 环境操作系统的版本
     */
    private String version;

    /**
     * 环境的扩展信息
     */
    private String extendInfo;

    /**
     * 更新时的协议类型
     */
    private String upgradeProtocolType;
}
