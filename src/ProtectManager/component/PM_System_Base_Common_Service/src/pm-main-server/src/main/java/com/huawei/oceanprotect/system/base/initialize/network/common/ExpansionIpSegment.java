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

import com.huawei.oceanprotect.system.base.initialize.network.beans.InitResource;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;
import lombok.NoArgsConstructor;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Pattern;

/**
 * 扩容Ip段信息
 *
 * @author swx1010572
 * @since 2021-06-11
 */
@Data
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class ExpansionIpSegment {
    /**
     * 起始IP
     */
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "please input correct ip startIp address")
    private String startIp;

    /**
     * ipv4/ipv6前缀
     */
    @Length(max = 15, message = "The length max of description is 15")
    private String subnetMask;

    /**
     * 结束IP
     */
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "please input correct ip endIp address")
    private String endIp;

    /**
     * 全参构造器
     *
     * @param startIp 起始IP
     * @param subnetMask ipv4/ipv6前缀
     * @param endIp 结束IP
     */
    public ExpansionIpSegment(@Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS,
        message = "please input correct ip startIp address") String startIp, String subnetMask,
        @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS,
            message = "please input correct ip endIp address") String endIp) {
        this.startIp = startIp;
        this.subnetMask = subnetMask;
        this.endIp = endIp;
    }

    /**
     * 获取构造新的InitResource
     *
     * @return 新的initResource
     */
    public InitResource castFromInitResource() {
        return new InitResource(startIp, endIp, subnetMask);
    }
}
