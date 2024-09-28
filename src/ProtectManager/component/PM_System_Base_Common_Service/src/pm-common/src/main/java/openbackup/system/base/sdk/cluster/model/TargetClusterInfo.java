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
package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.common.constants.IsmConstant;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Data;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * common模块LocalCluster
 *
 */
@Data
public class TargetClusterInfo {
    /* Cluster id */
    private Integer clusterId;

    /* Cluster name */
    private String clusterName;

    /* Cluster status;27: Online； 28：Offline */
    private Integer status;

    /* Trunking service IP */
    @NotNull
    @Size(max = IsmNumberConstant.TWO_HUNDRED_FIFTY_SIX, min = IsmNumberConstant.ONE,
        message = "The length of clusterIp is 1-256 characters")
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "The clusterIp is invalid")
    private String clusterIp;

    /* Cluster port */
    @NotNull(message = "The clusterPort cannot be null")
    @Max(IsmConstant.PORT_MAX)
    @Min(IsmConstant.PORT_MIN)
    private Integer clusterPort;

    /* IP address of the cluster protection engine */
    private String destVip;

    /* remote esn */
    private String remoteEsn;

    /* Cluster User Name */
    private String username;

    /* password */
    private String password;

    /* Verify flag */
    private String verifyFlag;

    /* Create time */
    private long createTime;

    /* Update time */
    private long lastUpdateTime;

    private int role;

    private boolean hasEnableManage;

    // 集群已使用容量 kb
    private double usedCapacity;

    // 集群容量 kb
    private double capacity;

    private String netPlaneName;
}
