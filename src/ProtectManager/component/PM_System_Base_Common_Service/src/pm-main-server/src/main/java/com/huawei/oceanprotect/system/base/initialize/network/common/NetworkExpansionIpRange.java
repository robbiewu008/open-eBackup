/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;

import java.util.List;

import javax.validation.Valid;

/**
 * 响应的ip段
 *
 * @author swx1010572
 * @since 2021-06-11
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class NetworkExpansionIpRange {
    @Valid
    private List<ExpansionIpSegment> backupPlane;

    @Valid
    private List<ExpansionIpSegment> archivePlane;


    @Valid
    private List<ExpansionIpSegment> copyPlane;
}
