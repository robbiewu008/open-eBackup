/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 归档到云的信息
 *
 * @author d00512967
 * @version [BCManager 8.0.0]
 * @since 2020-12-12
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
