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

import openbackup.system.base.common.constants.ProtocolPortConstant;
import openbackup.system.base.common.enums.StorageConnectTypeEnum;
import openbackup.system.base.common.model.storage.StorageRequest;
import openbackup.system.base.common.utils.VerifyUtil;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * DME 连通性请求体
 *
 * @author w00504341
 * @since 2020-12-19
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class DmeS3ConnectRequest {
    @JsonProperty("Url")
    private String url;

    @JsonProperty("Port")
    private int port;

    @JsonProperty("Username")
    private String userName;

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

    @JsonProperty("ProxyUserName")
    private String proxyUserName;

    @JsonProperty("ProxyUserPassword")
    private String proxyUserPass;

    @JsonProperty("UseHttps")
    private boolean isUseHttps;

    @JsonProperty("ProxyPort")
    private String proxyPort;

    private Integer connectType;

    private Integer cloudType;

    /**
     * DmeS3ConnectRequest
     *
     * @param request storageRequest
     */
    public DmeS3ConnectRequest(StorageRequest request) {
        this.setBucketName(request.getBucketName());
        this.setCertificate("");
        this.setPassword(request.getSk());
        this.setProxyEnable(request.isProxyEnable());
        this.setProxyUserName(request.getProxyUserName());
        this.setProxyHostName(request.getProxyHostName());
        this.setProxyUserPass(request.getProxyUserPwd());
        this.setUrl(request.getEndpoint());
        this.setUseHttps(request.isUseHttps());
        this.setUserName(request.getAk());
        this.setCloudType(request.getCloudType());
        this.setConnectType(request.getConnectType());
        if (request.isUseHttps()) {
            this.setCertificate(request.getCertName());
            this.setPort(ProtocolPortConstant.HTTPS_PORT);
        } else {
            this.setPort(ProtocolPortConstant.HTTP_PORT);
        }
        if (request.isAzureBlob()
            && request.getConnectType() == StorageConnectTypeEnum.STANDARD.getConnectType()) {
            this.setPort(request.getPort());
        }
        if (!VerifyUtil.isEmpty(request.getProxyHostName())) {
            int index = request.getProxyHostName().lastIndexOf(":");
            if (index != -1) {
                this.setProxyHostName(request.getProxyHostName().substring(0, index));
                this.setProxyPort(request.getProxyHostName().substring(index + 1));
            }
        }
    }
}
