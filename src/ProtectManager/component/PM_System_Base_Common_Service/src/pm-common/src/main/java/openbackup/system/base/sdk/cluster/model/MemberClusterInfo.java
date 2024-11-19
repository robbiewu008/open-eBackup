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

/**
 * member cluster vo
 *
 */
@Data
public class MemberClusterInfo {
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

    /* password */
    private String password;

    /* Create time */
    private long createTime;

    /* Update time */
    private long lastUpdateTime;

    private int role;

    private String netPlaneName;

    /**
     * 是否健康
     *
     * @return 是否健康
     */
    public boolean isHealth() {
        return status != null && status.equals(ClusterEnum.StatusEnum.ONLINE.getStatus());
    }
}
