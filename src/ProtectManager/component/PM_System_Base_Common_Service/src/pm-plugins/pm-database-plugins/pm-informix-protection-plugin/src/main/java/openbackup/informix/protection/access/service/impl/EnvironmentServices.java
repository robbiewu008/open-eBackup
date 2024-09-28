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
package openbackup.informix.protection.access.service.impl;

import openbackup.access.framework.resource.service.provider.UnifiedEnvironmentCheckProvider;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

/**
 * EnvironmentServices
 *
 */
@Service
@Slf4j
public class EnvironmentServices {
    private final UnifiedEnvironmentCheckProvider unifiedCheckProvider;

    private final ClusterEnvironmentService clusterEnvironmentService;

    /**
     * EnvironmentServices
     *
     * @param unifiedCheckProvider unifiedCheckProvider
     * @param clusterEnvironmentService clusterEnvironmentService
     */
    public EnvironmentServices(UnifiedEnvironmentCheckProvider unifiedCheckProvider,
        ClusterEnvironmentService clusterEnvironmentService) {
        this.unifiedCheckProvider = unifiedCheckProvider;
        this.clusterEnvironmentService = clusterEnvironmentService;
    }

    public UnifiedEnvironmentCheckProvider getUnifiedCheckProvider() {
        return unifiedCheckProvider;
    }

    public ClusterEnvironmentService getClusterEnvironmentService() {
        return clusterEnvironmentService;
    }
}
