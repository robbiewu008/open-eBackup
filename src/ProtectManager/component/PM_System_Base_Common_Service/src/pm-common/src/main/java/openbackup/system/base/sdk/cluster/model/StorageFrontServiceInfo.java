/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import com.fasterxml.jackson.annotation.JsonAlias;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Storage front service ips info
 *
 * @author p30001902
 * @since 2020-12-17
 */

@Data
public class StorageFrontServiceInfo {
    @JsonProperty("name")
    @JsonAlias("NAME")
    private String name;

    @JsonProperty("currentControllerId")
    @JsonAlias("CURRENTCONTROLLERID")
    private String currentControllerId;

    // 父端口类型
    @JsonProperty("homePortType")
    @JsonAlias("HOMEPORTTYPE")
    private String homePortType;

    @JsonProperty("ipV4Address")
    @JsonAlias("IPV4ADDR")
    private String ipV4Address;

    @JsonProperty("ipV6Address")
    @JsonAlias("IPV6ADDR")
    private String ipV6Address;
}
