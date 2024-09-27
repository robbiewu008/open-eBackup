/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.entity;

import openbackup.system.base.common.constants.IsmNumberConstant;

import lombok.Data;

/**
 * Replication Plan Entity
 *
 * @author l00272247
 * @since 2021-02-08
 */
@Data
public class ReplicationPlanEntity {
    // 保护计划对应的资源ID
    private String resourceId;

    // 目标集群ID
    private String targetClusterId;

    // 保护计划的复制策略
    private String policy;

    private String protectionPlanId;

    // 复制计划ID
    private String replicationPlanId;

    // 复制副本gn号
    private long gn = IsmNumberConstant.NEGATIVE_ONE;

    // 由于该资源已经被删除，但复制计划未被删除，需要做删除标记
    private Boolean needDeleted;
}
