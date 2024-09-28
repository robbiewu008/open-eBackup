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
package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;
import lombok.ToString;

/**
 * 存储设备的信息实体类
 *
 */
@ToString(exclude = "password")
@Getter
@Setter
public class RestoreStorageInfo {
    /**
     * 存储设备的ip或域名
     */
    @JsonProperty("ip")
    private String ip;

    /**
     * 端口
     */
    @JsonProperty("port")
    private String port;

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
     * 存储设备类型
     */
    @JsonProperty("storage_type")
    private String storageType;

    /**
     * 存储协议
     */
    @JsonProperty("protocol")
    private String protocol;

    /**
     * 设备esn
     */
    @JsonProperty("device_esn")
    private String deviceEsn;

    /**
     * 存储单元id
     */
    @JsonProperty("storage_id")
    private String storageId;
}
