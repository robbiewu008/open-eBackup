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
package openbackup.access.framework.resource.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import org.hibernate.validator.constraints.Length;

import java.util.List;
import java.util.Map;

import javax.validation.constraints.Max;

/**
 * 更新受保护环境信息DTO对象
 *
 */
@Data
public class BaseProtectedEnvironmentDto {
    // 受保护环境的uuid
    @Length(max = 64)
    private String uuid;

    // 受保护环境的名称
    @Length(max = 64)
    private String name;

    // 受保护环境的IP地址和域名
    @Length(max = 128)
    private String endpoint;

    // 受保护环境的端口
    @Max(65535)
    private int port;

    // 受保护环境的用户名
    @Length(max = 256)
    private String username;

    // 受保护环境的密码
    @Length(max = 2048)
    private String password;

    // 受保护环境的操作系统类型
    @Length(max = 32)
    private String osType;

    /**
     * 资源的扩展属性
     */
    private Map<String, String> extendInfo;

    /**
     * 是否同步agent主机名称
     */
    @JsonProperty("is_auto_synchronize_host_name")
    private String isAutoSynchronizeHostName;

    // 资源的依赖
    private Map<String, List<ProtectedResource>> dependencies;

    /**
     * 认证信息
     */
    private Authentication auth;
}
