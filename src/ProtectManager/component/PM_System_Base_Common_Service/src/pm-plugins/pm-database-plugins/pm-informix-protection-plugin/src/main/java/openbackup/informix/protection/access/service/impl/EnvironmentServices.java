/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.informix.protection.access.service.impl;

import openbackup.access.framework.resource.service.provider.UnifiedEnvironmentCheckProvider;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

/**
 * EnvironmentServices
 *
 * @author zwx951267
 * @version [DataBackup 1.5.0]
 * @since 2023-05-26
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
