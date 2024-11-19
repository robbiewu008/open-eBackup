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
package openbackup.system.base.bean;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 每一个设备下面的用户
 *
 */
@Setter
@Getter
public class DeviceUser {
    /**
     * 设备ESN
     */
    @JsonProperty("device_id")
    private String id;

    /**
     * 设备类型
     */
    @JsonProperty("device_type")
    private String deviceType;

    /**
     * 设备ip
     */
    @JsonProperty("ip")
    private String ip;

    /**
     * 设备port
     */
    @JsonProperty("port")
    private Integer port;

    /**
     * 用户名
     */
    @JsonProperty("username")
    private String username;

    /**
     * 密码
     */
    @JsonProperty("password")
    private String password;

    /**
     * 设备版本
     */
    @JsonProperty("device_version")
    private String deviceVersion;

    /**
     * 是否为默认用户
     */
    @JsonProperty("is_default")
    private boolean isDefaultUser = false;

    /**
     * 最后更新时间
     */
    @JsonProperty("last_update_time")
    private Long lastUpdateTime;

    /**
     * 角色，和member表中节点的角色一致
     */
    @JsonProperty("role")
    private Integer role;

    /**
     * 认证类型，0：业务认证，1：登录认证
     */
    @JsonProperty("auth_type")
    private Integer authType;
}
