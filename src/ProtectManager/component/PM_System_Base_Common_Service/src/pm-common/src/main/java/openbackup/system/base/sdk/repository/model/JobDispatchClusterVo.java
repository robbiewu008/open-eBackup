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
package openbackup.system.base.sdk.repository.model;

import lombok.Data;
import openbackup.system.base.common.utils.VerifyUtil;

import java.math.BigDecimal;
import java.util.List;
import java.util.Objects;

/**
 * nas存储库集群信息
 *
 */
@Data
public class JobDispatchClusterVo {
    private Integer clusterId;

    private String clusterName;

    private Integer status;

    private List<String> ip;

    private int clusterType;

    private Integer port;

    private String username;

    private int role;

    private String clusterIp;

    // 集群已使用容量 kb
    private BigDecimal usedCapacity;

    // 集群容量 kb
    private BigDecimal capacity;

    private String storageEsn;

    // 可使用容量阀值 0~100%
    private Integer availableCapacityRatio;

    // 备份存储单元添加方式 1：手动添加 2：自动添加（添加成员集群的时候自动添加）
    private Integer generatedType;

    // 备份存储单元在指定策略下的顺序
    private Integer strategyOrder;

    // 节点类型 DeployTypeEnum
    private String deployType;

    // 控制器数量
    private Integer controllerCount;

    // pm端口号
    private Integer pmPort;

    // pm在线状态
    private Integer pmStatus;

    /**
     * az信息，availableZoneId
     */
    private String availableZoneId;

    /**
     * 存储单元类型
     */
    private String deviceType;

    /**
     * 获取空闲空间
     *
     * @return 空闲空间
     */
    public BigDecimal getFreeCapacity() {
        if (Objects.isNull(capacity) || Objects.isNull(usedCapacity)) {
            return BigDecimal.ZERO;
        }
        return capacity.subtract(usedCapacity);
    }

    public Integer getControllerCount() {
        return VerifyUtil.isEmpty(controllerCount) ? 0 : controllerCount;
    }

    public Integer getPmPort() {
        return VerifyUtil.isEmpty(pmPort) ? 0 : pmPort;
    }

    public Integer getPmStatus() {
        return VerifyUtil.isEmpty(pmStatus) ? status : pmStatus;
    }
}
