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

import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.repository.model.BackupClusterVo;

import lombok.Getter;
import lombok.Setter;

import java.math.BigDecimal;

/**
 * 备份存储单元信息
 *
 * @author y30046482
 * @since 2023-06-26
 */
@Getter
@Setter
public class BackupStorageUnit {
    /* Cluster id */
    private Integer clusterId;

    /* Cluster name */
    private String clusterName;

    /* Cluster status;27: Online； 28：Offline */
    private Integer status;

    /* Trunking service IP */
    private String clusterIp;

    /* Cluster port */
    private Integer clusterPort;

    /* remote esn */
    private String remoteEsn;

    /* Cluster User Name */
    private String username;

    private int role;

    // 集群已使用容量 kb
    private BigDecimal usedCapacity;

    // 集群容量 kb
    private BigDecimal capacity;

    /* 默认是1 1是手动添加 2是自动添加 自动添加是指创建成员集群的时候自动创建的备份存储单元 */
    private Integer generatedType;

    // 节点类型 DeployTypeEnum
    private String deployType;

    // 控制器数量
    private Integer controllerCount;

    // pm端口号
    private Integer pmPort;

    /* pm Cluster status;27: Online； 28：Offline */
    private Integer pmStatus;

    /* 集群的az信息 */
    private String availableZoneId;

    /**
     * 转换为 BackupClusterVo
     *
     * @return BackupClusterVo
     */
    public BackupClusterVo toBackupClusterVo() {
        BackupClusterVo vo = new BackupClusterVo();
        vo.setClusterId(this.getClusterId());
        vo.setClusterName(this.getClusterName());
        vo.setStatus(this.getStatus());
        vo.setClusterIp(this.getClusterIp());
        vo.setPort(this.getClusterPort());
        vo.setStorageEsn(this.getRemoteEsn());
        vo.setUsername(this.getUsername());
        vo.setRole(this.getRole());
        vo.setGeneratedType(this.getGeneratedType());
        vo.setCapacity(this.getCapacity());
        vo.setUsedCapacity(this.getUsedCapacity());
        vo.setDeployType(this.getDeployType());
        vo.setControllerCount(this.getControllerCount());
        vo.setPmPort(this.getPmPort());
        vo.setPmStatus(this.pmStatus);
        vo.setClusterType(ClusterEnum.TypeEnum.REMOTE.getType());
        vo.setAvailableZoneId(this.getAvailableZoneId());
        if (this.getClusterIp().contains(",")) {
            vo.setIp(this.getClusterIp().substring(0, this.getClusterIp().indexOf(",")));
        } else {
            vo.setIp(this.getClusterIp());
        }
        return vo;
    }
}
