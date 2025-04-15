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

import static openbackup.system.base.common.constants.IsmNumberConstant.FOUR;
import static openbackup.system.base.common.constants.IsmNumberConstant.TWO_HUNDRED_FIFTY_SIX;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.util.CheckInputString;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

/**
 * 目标集群请求下发基类
 *
 */
@Data
public class TargetClusterRequest {
    @NotNull(message = "The clusterName cannot be null.")
    @NotEmpty(message = "The clusterName cannot be empty. ")
    @NotBlank(message = "The clusterName cannot be blank. ")
    @Length(max = TWO_HUNDRED_FIFTY_SIX, min = FOUR, message = "The length of cluster name is 4-256 characters")
    @Pattern(regexp = RegexpConstants.NAME_STR, message = "cluster name is invalid")
    private String clusterName;

    @NotNull(message = "The IP of cluster cannot be null")
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "cluster ip is invalid, not ipv4 or ipv6.")
    private String ip;

    private Integer port;

    private String username;

    // 可能不会下发密码，不在这里校验
    private String password;

    @NotNull(message = "The role cannot be null")
    @Min(IsmNumberConstant.ONE)
    @Max(IsmNumberConstant.EIGHT)
    private Integer role;

    private String netPlaneName;

    @CheckInputString(maxLen = 64, message = "availableZoneId is invalid.")
    private String availableZoneId;

    // 默认会转发到其他管理集群进行同步修改、添加
    @JsonProperty("syncToRemote")
    private boolean shouldSyncToRemote = true;

    @Max(Integer.MAX_VALUE)
    @Min(IsmNumberConstant.TWO)
    private Integer backupUnitId;

    // 设备类型，非必填，复制集群，备份集群和管理集群也用这个类
    @Length(max = 64)
    private String deviceType;

    // 设备id，本地盘服务器的备份存储设备必填，其他非必填
    private String deviceId;

    // hcs集群的对外域名
    private String domain;

    // 复制集群类型
    private int replicationClusterType;
}
