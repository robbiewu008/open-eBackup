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
