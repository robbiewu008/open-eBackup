/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
 * 归档网络信息
 *
 * @author l00347293
 * @since 2020-12-08
 */
@Data
@Slf4j
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class ArchiveNetworkConfig {
    List<LogicPortDto> logicPorts;

    private List<NodeNetworkInfoRequest> pacificInitNetWorkInfoList;
}