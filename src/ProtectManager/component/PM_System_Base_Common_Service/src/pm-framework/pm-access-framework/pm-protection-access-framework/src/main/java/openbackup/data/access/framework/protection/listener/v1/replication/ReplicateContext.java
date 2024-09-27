/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.listener.v1.replication;

import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.replication.IReplicateContext;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.resource.model.ResourceEntity;

import lombok.Builder;
import lombok.Getter;
import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.springframework.beans.factory.annotation.Autowired;

import java.util.List;

/**
 * Replicate Context
 *
 * @author l00272247
 * @since 2020-11-21
 */
@Getter
@Builder
@Slf4j
public class ReplicateContext implements IReplicateContext {
    @Autowired
    private BackupObject backupObject;

    @Autowired
    private ResourceEntity resourceEntity;

    @Autowired
    private PolicyBo policy;

    @Autowired
    private TargetClusterVo targetCluster;

    private List<String> sameChainCopies;

    private RMap<String, String> context;

    private String requestId;

    private String cachedToken;
}
