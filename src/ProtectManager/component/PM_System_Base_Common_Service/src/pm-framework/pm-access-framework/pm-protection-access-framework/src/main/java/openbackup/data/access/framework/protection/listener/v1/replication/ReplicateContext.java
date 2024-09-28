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
