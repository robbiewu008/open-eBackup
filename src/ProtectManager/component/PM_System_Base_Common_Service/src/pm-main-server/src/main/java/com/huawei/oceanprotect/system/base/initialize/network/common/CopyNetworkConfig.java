/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.sdk.devicemanager.request.NodeNetworkInfoRequest;

import java.util.List;

/**
 * 复制网络信息
 *
 * @author swx1010572
 * @version: [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-26
 */
@Data
@Slf4j
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class CopyNetworkConfig {
    List<LogicPortDto> logicPorts;

    private List<NodeNetworkInfoRequest> pacificInitNetWorkInfoList;
}