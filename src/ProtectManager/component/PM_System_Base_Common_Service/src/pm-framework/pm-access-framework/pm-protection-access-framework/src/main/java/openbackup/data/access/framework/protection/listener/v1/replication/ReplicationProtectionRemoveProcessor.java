/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.listener.v1.replication;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.model.ResourceEntity;

/**
 * Protection Remove Processor
 *
 * @author l00272247
 * @since 2021-01-05
 */
public interface ReplicationProtectionRemoveProcessor extends DataProtectionProvider<String> {
    /**
     * process protection remove event
     *
     * @param resourceEntity resource entity
     * @param targetCluster targetCluster
     */
    void process(ResourceEntity resourceEntity, TargetClusterVo targetCluster);

    /**
     * process protection remove event
     *
     * @param resourceEntity resource entity
     * @param detail 备份存储单元组
     */
    void process(ResourceEntity resourceEntity, NasDistributionStorageDetail detail);
}
