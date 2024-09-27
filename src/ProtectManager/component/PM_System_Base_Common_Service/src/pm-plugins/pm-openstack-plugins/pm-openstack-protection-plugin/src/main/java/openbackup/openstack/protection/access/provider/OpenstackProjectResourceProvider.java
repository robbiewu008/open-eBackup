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
package openbackup.openstack.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.dto.ResourceScanParam;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.OptionalUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 项目资源Provider
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-29
 */
@Slf4j
@Component
public class OpenstackProjectResourceProvider implements ResourceProvider {
    private final OpenstackResourceScanProvider openstackResourceScanProvider;
    private final ResourceService resourceService;
    private final ResourceExtendInfoService resourceExtendInfoService;

    public OpenstackProjectResourceProvider(OpenstackResourceScanProvider openstackResourceScanProvider,
        ResourceService resourceService, ResourceExtendInfoService resourceExtendInfoService) {
        this.openstackResourceScanProvider = openstackResourceScanProvider;
        this.resourceService = resourceService;
        this.resourceExtendInfoService = resourceExtendInfoService;
    }

    @Override
    public List<ProtectedResource> scan(ProtectedResource projectResource) {
        log.info("scan project resource start. projectId:{}", projectResource.getUuid());
        ResourceScanParam scanParam = new ResourceScanParam();
        setProviderEnv(projectResource, scanParam);
        List<ProtectedResource> projectResources = Collections.singletonList(projectResource);
        scanParam.setProjectResources(projectResources);
        setDomainAuth(scanParam, projectResource);
        // 通过agent扫描项目下子资源
        List<ProtectedResource> scannedResources =
            openstackResourceScanProvider.scanByAgent(openstackResourceScanProvider::scanProjectSubResource, scanParam);
        log.info("scan project resource finished. projectId:{}, resources:{}", projectResource.getUuid(),
            scannedResources.size());
        if (resourceService.getBasicResourceById(projectResource.getUuid()).isPresent()) {
            resourceExtendInfoService.saveOrUpdateExtendInfo(projectResource.getUuid(),
                projectResource.getExtendInfo());
        }
        return scannedResources;
    }

    @Override
    public boolean applicable(ProtectedResource resource) {
        return ResourceSubTypeEnum.OPENSTACK_PROJECT.equalsSubType(resource.getSubType());
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
    }

    private void setProviderEnv(ProtectedResource resource, ResourceScanParam scanParam) {
        resourceService.getResourceById(resource.getRootUuid())
            .flatMap(OptionalUtil.match(ProtectedEnvironment.class))
            .ifPresent(scanParam::setEnvironment);
    }

    private void setDomainAuth(ResourceScanParam scanParam, ProtectedResource projectResource) {
        Map<String, Authentication> domainAuthMap = new HashMap<>();
        domainAuthMap.put(projectResource.getParentUuid(),
            resourceService.getResourceById(projectResource.getParentUuid())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST,
                    "Openstack project(id: " + projectResource.getUuid() + ")'s domain resource not exist"))
                .getAuth());
        scanParam.setDomainAuthMap(domainAuthMap);
    }
}
