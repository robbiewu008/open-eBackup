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
package openbackup.system.base.sdk.anti.model;

import lombok.Data;

import org.hibernate.validator.constraints.Range;

import java.util.List;

import javax.validation.constraints.Min;

/**
 * 功能描述
 *
 */
@Data
public class AirGapDeviceShowPageReq {
    /**
     * 页码
     */
    @Min(0)
    private int pageNo;

    /**
     * 页大小
     */
    @Range(min = 1, max = 200)
    private int pageSize;

    /**
     * 策略id
     */
    private String policyId;

    /**
     * 设备名称
     */
    private String name;

    /**
     * 设备ESN
     */
    private String esn;

    /**
     * 设备在线状态 在线1，离线0
     */
    private List<String> linkStatus;

    /**
     * 复制链路状态 连通open, 断开close,未知unknown
     */
    private List<String> replicationLinkStatus;

    /**
     * 策略开启状态 enable应用中,disable未应用,invalid已失效
     */
    private List<String> policyStatus;

    /**
     * 策略名称
     */
    private String policyName;

    /**
     * 集群名称 不传递或传递为null或传递为空字符串，则默认查询全部
     */
    private String clusterName;
}
