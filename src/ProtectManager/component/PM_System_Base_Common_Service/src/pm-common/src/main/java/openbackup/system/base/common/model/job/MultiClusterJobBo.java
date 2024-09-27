/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.common.model.job;

import openbackup.system.base.util.BeanTools;

import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * MultiClusterJobBo
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-12-22
 */
@Getter
@Setter
@NoArgsConstructor
public class MultiClusterJobBo extends JobBo {
    private Integer clusterId;

    private int role;

    /**
     * MultiClusterJobBo
     *
     * @param jobBo jobBo
     * @param clusterId clusterId
     * @param role role
     */
    public MultiClusterJobBo(JobBo jobBo, Integer clusterId, int role) {
        BeanTools.copy(jobBo, this);
        setClusterId(clusterId);
        setRole(role);
    }
}
