/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;
import openbackup.system.base.sdk.devicemanager.request.NodeNetworkInfoRequest;

import java.util.List;

import javax.validation.Valid;
import javax.validation.constraints.Size;

/**
 * 备份网络信息
 *
 * @author l00347293
 * @since 2020-12-08
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class BackupNetworkConfig {
    @Valid
    @Size(min = 1, message = "The backup network logic port is configured with at least one property")
    private List<LogicPortDto> logicPorts;

    @Valid
    @Size(min = 1, message = "The backup network pacific init network info is configured with at least one property")
    private List<NodeNetworkInfoRequest> pacificInitNetWorkInfoList;
}
