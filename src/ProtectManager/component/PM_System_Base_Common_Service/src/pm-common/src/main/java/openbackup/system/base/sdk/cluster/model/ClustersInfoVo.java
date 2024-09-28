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

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.List;

/**
 * Clusters info vo
 *
 */
@Data
public class ClustersInfoVo {
    private Integer clusterId;

    private String clusterName;

    private Integer status;

    private String ip;

    private int clusterType;

    private Integer port;

    private String username;

    private int role;

    @JsonProperty("enableManage")
    private boolean hasEnableManage;

    private String clusterIp;

    // 集群已使用容量 kb
    private BigDecimal usedCapacity;

    // 集群容量 kb
    private BigDecimal capacity;

    private String storageEsn;

    // 内部通信网络平面
    private String internalCommunicateNetPlane;

    // 内部通信网络平面设置状态
    private Integer netPlaneSettingStatus;

    private List<ClustersRelationUserInfo> authUserList = new ArrayList<>();

    // 备份存储单元添加方式 1：手动添加 2：自动添加（添加成员集群的时候自动添加）
    private Integer generatedType;

    // 型号
    private String mode;

    private String availableZoneId;

    private String deviceType;

    private String domain;

    private int replicationClusterType;
}
