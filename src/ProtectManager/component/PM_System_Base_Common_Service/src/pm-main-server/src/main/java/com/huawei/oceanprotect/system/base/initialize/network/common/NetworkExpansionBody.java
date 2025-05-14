/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;
import openbackup.system.base.sdk.system.model.StorageAuth;

import org.hibernate.validator.constraints.Range;

import java.util.List;

import javax.validation.Valid;

/**
 * 扩容信息;
 *
 * @author swx1010572
 * @since 2021-06-11
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class NetworkExpansionBody {
    /**
     * 存储用户名、密码
     */
    @Valid
    private StorageAuth storageAuth;

    @Valid
    private List<ExpansionIpSegment> backupPlane;

    @Valid
    private List<ExpansionIpSegment> archivePlane;

    @Valid
    private List<ExpansionIpSegment> copyPlane;

    @Range(min = 2, max = 32, message = "Please input controller in range [2, 32]")
    private int controller;
}
