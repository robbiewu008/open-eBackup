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
package openbackup.data.access.framework.core.entity;

import lombok.Data;
import openbackup.system.base.common.constants.IsmNumberConstant;

/**
 * Replication Plan Entity
 *
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
