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
