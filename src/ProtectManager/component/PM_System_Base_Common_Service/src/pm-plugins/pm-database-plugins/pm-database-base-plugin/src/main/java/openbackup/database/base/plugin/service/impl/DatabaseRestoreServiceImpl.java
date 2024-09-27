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
package openbackup.database.base.plugin.service.impl;

import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.service.DatabaseRestoreService;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.util.Objects;

/**
 * 数据库恢复服务
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-02-08
 */
@Service
@Slf4j
public class DatabaseRestoreServiceImpl implements DatabaseRestoreService {
    @Override
    public void checkDeployOperatingSystem(String sourceDeployOs, String targetDeployOs) {
        if (!Objects.equals(sourceDeployOs, targetDeployOs)) {
            log.error("The operating systems deployed at the source and target are inconsistent. source:{}, target:{}.",
                sourceDeployOs, targetDeployOs);
            throw new LegoCheckedException(DatabaseErrorCode.RESTORE_OS_INCONSISTENT,
                "Deploy operating system inconsistent.");
        }
    }

    @Override
    public void checkResourceSubType(String sourceSubType, String targetSubType) {
        if (!Objects.equals(sourceSubType, targetSubType)) {
            log.error("The resource type on the source and target is inconsistent. source:{}, target:{}.",
                sourceSubType, targetSubType);
            throw new LegoCheckedException(DatabaseErrorCode.RESTORE_RESOURCE_TYPE_INCONSISTENT,
                "Resource type is inconsistent.");
        }
    }

    @Override
    public void checkClusterType(String sourceClusterType, String targetClusterType) {
        if (!Objects.equals(sourceClusterType, targetClusterType)) {
            log.error("The cluster type at the source and target is inconsistent. source:{}, target:{}.",
                sourceClusterType, targetClusterType);
            throw new LegoCheckedException(DatabaseErrorCode.RESTORE_TARGET_RESOURCE_TYPE_INCONSISTENT,
                "Cluster type is inconsistent.");
        }
    }

    @Override
    public void checkVersion(String sourceVersion, String targetVersion) {
        if (!Objects.equals(sourceVersion, targetVersion)) {
            log.error("The version of the source and target is inconsistent. source:{}, target:{}.", sourceVersion,
                targetVersion);
            throw new LegoCheckedException(DatabaseErrorCode.RESTORE_RESOURCE_VERSION_INCONSISTENT,
                "Version is inconsistent.");
        }
    }
}
