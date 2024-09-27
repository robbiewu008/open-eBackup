/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * VM hardware信息
 *
 * @author h30003246
 * @since 2020-12-04
 */
@Data
public class VMWareHardware {
    @JsonProperty("num_cpu")
    private Integer numCpu;

    @JsonProperty("num_cores_per_socket")
    private Integer numCoresPerSocket;

    @JsonProperty("memory")
    private Integer memory;

    @JsonProperty("controller")
    private List<String> controller;
}
