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
package openbackup.oceanbase.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.oceanbase.common.dto.OBClusterInfo;
import openbackup.oceanbase.common.util.OceanBaseUtils;
import openbackup.oceanbase.service.OceanBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Optional;

/**
 * 功能描述
 *
 */
@Slf4j
@Component
public class OceanBaseClusterDeleteProvider implements ResourceProvider {
    private final OceanBaseService oceanBaseService;

    /**
     * OceanBaseClusterDeleteProvider
     *
     * @param oceanBaseService oceanBaseService
     */
    public OceanBaseClusterDeleteProvider(OceanBaseService oceanBaseService) {
        this.oceanBaseService = oceanBaseService;
    }

    @Override
    public boolean applicable(ProtectedResource resourceSubType) {
        return ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType().equals(resourceSubType.getSubType());
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {

    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {

    }

    @Override
    public ResourceDeleteContext preHandleDelete(ProtectedResource resource) {
        ResourceDeleteContext resourceDeleteContext = ResourceDeleteContext.defaultValue();
        log.info("[OceanBase Delete] delete start, cluster uuid: {}", resource.getUuid());
        Optional<ProtectedResource> resOptional = oceanBaseService.getResourceById(resource.getUuid());
        if (!resOptional.isPresent()) {
            log.warn("resource {} not exist", resource.getUuid());
            return resourceDeleteContext;
        }

        // 资源离线状态直接返回
        ProtectedEnvironment env = oceanBaseService.getEnvironmentById(resource.getUuid());
        if (env == null || StringUtils.equals(env.getLinkStatus(), LinkStatusEnum.OFFLINE.getStatus().toString())) {
            log.warn("OceanBase cluster {} is offline", resource.getUuid());
            return resourceDeleteContext;
        }

        OBClusterInfo obClusterInfo = Optional.ofNullable(OceanBaseUtils.readExtendClusterInfo(resource))
            .orElse(new OBClusterInfo());
        // 解除数据仓持续挂载
        try {
            oceanBaseService.umountDataRepo(obClusterInfo, resource);
        } catch (LegoCheckedException e) {
            log.error("oceanbase Execute umount repo task failed.", e);
        }

        // 删除持久仓白名单
        try {
            oceanBaseService.removeDataRepoWhiteListOfResource(resOptional.get().getUuid());
        } catch (LegoUncheckedException | FeignException e) {
            log.error("oceanbase Execute removeDataRepoWhiteList task failed.", e);
        }
        log.info("Execute umount repo task completed, resource id: {}.", resource.getUuid());
        return ResourceDeleteContext.defaultValue();
    }
}
