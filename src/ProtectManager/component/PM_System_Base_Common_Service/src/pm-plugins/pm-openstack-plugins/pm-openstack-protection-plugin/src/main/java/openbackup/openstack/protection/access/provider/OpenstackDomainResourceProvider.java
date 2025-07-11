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

import openbackup.openstack.protection.access.common.OpenstackCommonService;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.constant.OpenstackDomainVisibleEnum;
import openbackup.openstack.protection.access.dto.ResourceScanParam;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.OptionalUtil;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * 域资源Provider
 *
 */
@Slf4j
@Component
public class OpenstackDomainResourceProvider implements ResourceProvider {
    private final ResourceService resourceService;
    private final OpenstackResourceScanProvider openstackResourceScanProvider;
    private final OpenstackCommonService openStackCommonService;


    public OpenstackDomainResourceProvider(ResourceService resourceService,
        OpenstackResourceScanProvider openstackResourceScanProvider, OpenstackCommonService openStackCommonCheck) {
        this.resourceService = resourceService;
        this.openstackResourceScanProvider = openstackResourceScanProvider;
        this.openStackCommonService = openStackCommonCheck;
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        if (isScanUpdate(resource)) {
            return;
        }
        openStackCommonService.checkOpenStackAndDomainMaxNum();
        resource.setSourceType(ResourceConstants.SOURCE_TYPE_REGISTER);
    }

    @Override
    public List<ProtectedResource> scan(ProtectedResource domainResource) {
        log.info("scan domain resource start. domainId:{}", domainResource.getUuid());
        ResourceScanParam scanParam = new ResourceScanParam();
        setProviderEnv(domainResource, scanParam);
        List<ProtectedResource> domainResources = Collections.singletonList(domainResource);
        scanParam.setDomainResources(domainResources);
        Map<String, Authentication> domainAuthMap = domainResources.stream()
            .collect(Collectors.toMap(ProtectedResource::getUuid, ProtectedResource::getAuth));
        scanParam.setDomainAuthMap(domainAuthMap);
        // 通过agent扫描domain下子资源
        List<ProtectedResource> scannedResources = openstackResourceScanProvider.scanByAgent(
            openstackResourceScanProvider::scanDomainSubResource, scanParam);
        computeDomainProjectNum(domainResource, scanParam.getDomainProjectCountMap());
        scannedResources.add(domainResource);
        log.info("scan domain resource finished. domainId:{}, resources:{}",
            domainResource.getUuid(), scannedResources.size());
        return scannedResources;
    }

    @Override
    public boolean applicable(ProtectedResource resource) {
        return ResourceSubTypeEnum.OPENSTACK_DOMAIN.equalsSubType(resource.getSubType());
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        return true;
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
    }

    private void computeDomainProjectNum(ProtectedResource domainResource, Map<String, Integer> domainProjectCountMap) {
        Integer projectCount = domainProjectCountMap.getOrDefault(domainResource.getUuid(), 0);
        domainResource.getExtendInfo().put(OpenstackConstant.PROJECT_COUNT, projectCount.toString());
    }


    private void setProviderEnv(ProtectedResource resource, ResourceScanParam scanParam) {
        resourceService.getResourceById(resource.getRootUuid())
            .flatMap(OptionalUtil.match(ProtectedEnvironment.class))
            .ifPresent(scanParam::setEnvironment);
    }

    private boolean isScanUpdate(ProtectedResource resource) {
        return Objects.equals(resource.getExtendInfo().get(OpenstackConstant.VISIBLE),
            OpenstackDomainVisibleEnum.INVISIBLE.getCode());
    }
}
