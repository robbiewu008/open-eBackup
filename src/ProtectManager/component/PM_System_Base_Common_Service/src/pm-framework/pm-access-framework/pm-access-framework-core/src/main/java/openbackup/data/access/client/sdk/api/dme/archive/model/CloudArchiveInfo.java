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
package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 归档到云的信息
 *
 */
@Data
public class CloudArchiveInfo {
    @JsonProperty("Url")
    private String url;

    @JsonProperty("Port")
    private Integer port;

    @JsonProperty("Username")
    private String username;

    @JsonProperty("Password")
    private String password;

    @JsonProperty("Certificate")
    private String certificate;

    @JsonProperty("BucketName")
    private String bucketName;

    @JsonProperty("ProxyEnable")
    private boolean isProxyEnable;

    @JsonProperty("ProxyHostName")
    private String proxyHostName;

    @JsonProperty("ProxyPort")
    private String proxyPort;

    @JsonProperty("ProxyUserName")
    private String proxyUserName;

    @JsonProperty("ProxyUserPassword")
    private String proxyUserPassword;

    @JsonProperty("SpeedUpEnable")
    private boolean isSpeedUpEnable;

    @JsonProperty("SpeedUpMethod")
    private String speedUpMethod;

    @JsonProperty("UseHttps")
    private boolean isUseHttps;

    /**
     * 连接模式
     */
    private Integer connectType;

    /**
     * 云存储类型
     */
    private Integer cloudType;
}
