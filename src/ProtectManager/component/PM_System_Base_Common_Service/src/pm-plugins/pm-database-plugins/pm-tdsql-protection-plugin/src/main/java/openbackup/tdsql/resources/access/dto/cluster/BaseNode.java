/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.dto.cluster;

import lombok.Data;

/**
 * Scheduler节点
 *
 * @author z30047175
 * @since 2023-05-23
 */
@Data
public class BaseNode {
    /**
     * 节点类型 Scheduler节点
     */
    private String nodeType;

    /**
     * 代理主机uuid
     */
    private String parentUuid;

    /**
     * 业务ip
     */
    private String ip;

    /**
     * linkStatus
     */
    private String linkStatus;
}
