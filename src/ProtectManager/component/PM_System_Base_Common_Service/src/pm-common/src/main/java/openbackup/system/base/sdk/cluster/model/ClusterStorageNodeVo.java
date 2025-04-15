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

import lombok.Data;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.infrastructure.enums.InfrastructureEnums;
import openbackup.system.base.sdk.infrastructure.model.beans.NodeDetail;

/**
 * Storage nodes vo
 *
 */
@Data
public class ClusterStorageNodeVo {
    private String nodeId;

    private String nodeName;

    private Integer nodeRole;

    private String managementIPv4;

    private String managementIPv6;

    private Integer status;

    // 数据备份引擎IP
    private String backupEngineIp;

    // 数据利用引擎IP
    private String deeEngineIp;

    // 数据归档引擎IP
    private String archiveEngineIp;

    // 复制网络ip
    private String copyEngineIp;

    // 健康状态
    private String healthStatus;

    /**
     * 通过nodeDetail给自己的相关属性赋值
     *
     * @param nodeDetail nodeDetail
     */
    public void buildFromNodeDetail(NodeDetail nodeDetail) {
        this.nodeId = nodeDetail.getNodeName();
        this.nodeName = nodeDetail.getNodeName();
        // nodeDetail中的ip都没有区分ipv4和ipv6
        this.managementIPv4 = nodeDetail.getManagementAddress();
        if (InfrastructureEnums.NodeStatusEnums.READY.getValue().equals(nodeDetail.getNodeStatus())) {
            this.healthStatus = ClusterEnum.StorageStatusEnum.STATUS_RUNNING.getStatus();
        }
    }
}
