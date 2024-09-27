/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tidb.resources.access.model;

import lombok.Getter;
import lombok.Setter;

/**
 * cluster Info model
 *
 * @author w00426202
 * @since 2023-07-24
 */
@Getter
@Setter
public class ClusterInfo {
    /**
     * 集群id
     */
    private String id;

    /**
     * 集群角色
     */
    private String role;

    /**
     * 集群host
     */
    private String host;

    /**
     * 集群节点状态
     */
    private String status;

    /**
     * 集群节点uuid
     */
    private String hostManagerResourceUuid;
}
