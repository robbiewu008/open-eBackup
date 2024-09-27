/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.devicemanager.entity;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;
import lombok.ToString;

/**
 * pacific节点名称及其对应性能监控id
 *
 * @author x30046484
 * @since 2024-03-01
 */
@Getter
@Setter
@ToString
public class PacificServerInfo {
    private Integer id;
    private String name;
    @JsonProperty("management_ip")
    private String managementIp;

    private Integer status;
}
