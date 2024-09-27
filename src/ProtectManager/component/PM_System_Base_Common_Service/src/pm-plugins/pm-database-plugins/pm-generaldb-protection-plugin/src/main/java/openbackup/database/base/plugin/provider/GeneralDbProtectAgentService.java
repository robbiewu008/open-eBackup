/*
 *
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

package openbackup.database.base.plugin.provider;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.util.GeneralDbUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.stream.Collectors;

/**
 * 通用数据库AgentSelector
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-28
 */
@Component
public class GeneralDbProtectAgentService {
    private final ResourceService resourceService;

    public GeneralDbProtectAgentService(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    /**
     * 查找endpoint
     *
     * @param protectedResource 资源
     * @return endpoint
     */
    public List<Endpoint> select(ProtectedResource protectedResource) {
        ProtectedResource currentResource = protectedResource;
        if (VerifyUtil.isEmpty(protectedResource.getDependencies())) {
            currentResource = resourceService.getResourceById(protectedResource.getUuid())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "resource can not found"));
        }
        return GeneralDbUtil.getHosts(currentResource)
            .stream()
            .map(e -> new Endpoint(e.getUuid(), e.getEndpoint(), e.getPort()))
            .collect(Collectors.toList());
    }
}
