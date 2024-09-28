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

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.constant.OpenstackDomainVisibleEnum;
import openbackup.openstack.protection.access.dto.ResourceScanParam;
import openbackup.openstack.protection.access.dto.VolInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.OptionalUtil;

import com.alibaba.fastjson.JSON;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

import java.io.File;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.TreeSet;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * OpenstackResourceScanProvider
 *
 */
@Slf4j
@Component
public class OpenstackResourceScanProvider implements EnvironmentProvider {
    private final AgentUnifiedService agentService;
    private final ResourceService resourceService;
    private final ProtectedEnvironmentRetrievalsService envRetrievalsService;
    private final EnvironmentCheckProvider environmentCheckProvider;
    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    public OpenstackResourceScanProvider(AgentUnifiedService agentService, ResourceService resourceService,
        ProtectedEnvironmentRetrievalsService envRetrievalsService,
        @Qualifier("unifiedEnvironmentCheckProvider") EnvironmentCheckProvider environmentCheckProvider,
        @Qualifier("unifiedConnectionCheckProvider") ResourceConnectionCheckProvider resourceConnectionCheckProvider) {
        this.agentService = agentService;
        this.resourceService = resourceService;
        this.envRetrievalsService = envRetrievalsService;
        this.environmentCheckProvider = environmentCheckProvider;
        this.resourceConnectionCheckProvider = resourceConnectionCheckProvider;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.OPENSTACK_CONTAINER.equalsSubType(subType);
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        ResourceScanParam resourceScanParam = new ResourceScanParam();
        resourceScanParam.setEnvironment(environment);
        return scanByAgent(this::scanResourceByAgent, resourceScanParam);
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("start check openstack environment");
        environment.setScanInterval(OpenstackConstant.SCAN_INTERVAL);
        environmentCheckProvider.check(environment);
    }

    @Override
    public void validate(ProtectedEnvironment environment) {
        log.info("start check openstack environment health.");
        environment.setExtendInfoByKey(OpenstackConstant.HEALTH_CHECK, Boolean.TRUE.toString());
        resourceConnectionCheckProvider.checkConnection(environment);
        log.info("finish check openstack environment health.");
    }

    List<ProtectedResource> scanByAgent(Function<ResourceScanParam, List<ProtectedResource>> scanMethod,
        ResourceScanParam resourceScanParam) {
        List<ProtectedEnvironment> agents = getAllAgents(resourceScanParam.getEnvironment());
        LegoCheckedException exception = new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR,
            "Resource scan failed.");
        for (ProtectedResource agent : agents) {
            try {
                resourceScanParam.setEndpoint(getAgentEndpoint(agent.getUuid()));
                return scanMethod.apply(resourceScanParam);
            } catch (LegoCheckedException e) {
                log.warn("Scan resources by agent:{} failed.", agent.getUuid(), ExceptionUtil.getErrorMessage(e));
                exception = e;
            }
        }
        throw exception;
    }

    private List<ProtectedResource> scanResourceByAgent(ResourceScanParam resourceScanParam) {
        Endpoint endpoint = resourceScanParam.getEndpoint();
        ProtectedEnvironment environment = resourceScanParam.getEnvironment();
        log.info("start scan openstack environment, agentId:{}, envId:{}", endpoint.getId(), environment.getUuid());
        /* 扫描domain资源 */
        List<ProtectedResource> domainResources = doScanResources(endpoint, Lists.newArrayList(),
            OpenstackConstant.RES_TYPE_DOMAIN, environment);
        List<ProtectedResource> visibleDomainResource = getVisibleDomainResource(environment);
        List<ProtectedResource> resources = Lists.newArrayList(domainResources);
        fillDomainProperties(domainResources, visibleDomainResource, environment);
        if (CollectionUtils.isEmpty(visibleDomainResource)) {
            log.warn("no visible domain from env. envId:{}", environment.getUuid());
            return resources;
        }
        resourceScanParam.setDomainResources(visibleDomainResource);
        Map<String, Authentication> domainAuthMap = visibleDomainResource.stream()
            .collect(Collectors.toMap(ProtectedResource::getUuid, ProtectedResource::getAuth));
        resourceScanParam.setDomainAuthMap(domainAuthMap);
        resources.addAll(scanDomainSubResource(resourceScanParam));
        fillDomainProjectCount(resourceScanParam, domainResources);
        log.info("finish scan openstack environment, agentId:{}, envId:{}", endpoint.getId(), environment.getUuid());
        return resources;
    }

    private void fillDomainProjectCount(ResourceScanParam resourceScanParam,
        List<ProtectedResource> visibleDomainResource) {
        visibleDomainResource.forEach(domainResource -> {
            String projectCount = resourceScanParam.getDomainProjectCountMap()
                .getOrDefault(domainResource.getUuid(), LegoNumberConstant.ZERO).toString();
            domainResource.getExtendInfo().put(OpenstackConstant.PROJECT_COUNT, projectCount);
        });
    }

    List<ProtectedResource> scanDomainSubResource(ResourceScanParam resourceScanParam) {
        ProtectedEnvironment environment = resourceScanParam.getEnvironment();
        List<ProtectedResource> domainResources = resourceScanParam.getDomainResources();
        List<List<Application>> domainAppLists = Lists.partition(
            convertToApps(domainResources, resourceScanParam.getDomainAuthMap(), environment), 1);
        List<ProtectedResource> projectResources = Lists.newArrayList();
        /* 扫描project资源 */
        domainAppLists.forEach(domainAppList -> projectResources.addAll(doScanResources(resourceScanParam.getEndpoint(),
            domainAppList, OpenstackConstant.RES_TYPE_PROJECT, environment))
        );
        fillProjectProperties(projectResources, environment, resourceScanParam.getDomainProjectCountMap());
        resourceScanParam.setProjectResources(projectResources);
        List<ProtectedResource> cloudServerResources = scanProjectSubResource(resourceScanParam);
        List<ProtectedResource> resources = Lists.newArrayList();
        resources.addAll(projectResources);
        resources.addAll(cloudServerResources);
        return resources;
    }

    List<ProtectedResource> scanProjectSubResource(ResourceScanParam resourceScanParam) {
        ProtectedEnvironment environment = resourceScanParam.getEnvironment();
        Endpoint endpoint = resourceScanParam.getEndpoint();
        List<ProtectedResource> projectResources = resourceScanParam.getProjectResources();
        List<List<Application>> projectAppLists = Lists.partition(
            convertToApps(projectResources, resourceScanParam.getDomainAuthMap(), environment), 1);
        List<ProtectedResource> cloudServerResources = Lists.newArrayList();
        List<ProtectedResource> cloudServerDiskResources = Lists.newArrayList();
        // 云服务器硬盘类型，用于磁盘恢复新建卷
        List<ProtectedResource> volumeTypeResources = Lists.newArrayList();
        // 云服务器网络，用于虚拟机恢复
        List<ProtectedResource> networkResources = Lists.newArrayList();
        // 虚拟机类型，用于虚拟机恢复
        List<ProtectedResource> flavorResources = Lists.newArrayList();
        // 虚拟机可用区域，用于虚拟机恢复
        List<ProtectedResource> availabilityZoneResources = Lists.newArrayList();
        /* 扫描云主机和硬盘、硬盘类型、网络、flavor */
        projectAppLists.forEach(projectAppList -> {
            cloudServerResources.addAll(
                doScanResources(endpoint, projectAppList, OpenstackConstant.RES_TYPE_SERVER, environment));
            availabilityZoneResources.addAll(
                doScanResources(endpoint, projectAppList, OpenstackConstant.RES_TYPE_AVAILABILITY_ZONE, environment));
            cloudServerDiskResources.addAll(
                doScanResources(endpoint, projectAppList, OpenstackConstant.RES_TYPE_VOLUME, environment));
            volumeTypeResources.addAll(
                doScanResources(endpoint, projectAppList, OpenstackConstant.RES_TYPE_VOLUME_TYPE, environment));
            networkResources.addAll(
                doScanResources(endpoint, projectAppList, OpenstackConstant.PROJECT_NETWORK, environment));
            flavorResources.addAll(
                doScanResources(endpoint, projectAppList, OpenstackConstant.PROJECT_FLAVOR, environment));
        });
        Map<String, ProtectedResource> projectMap = projectResources.stream()
            .collect(Collectors.toMap(ProtectedResource::getUuid, Function.identity()));
        fillProjectExtend(projectResources, volumeTypeResources, OpenstackConstant.RES_TYPE_VOLUME_TYPE);
        fillProjectExtend(projectResources, flavorResources, OpenstackConstant.PROJECT_FLAVOR);
        fillProjectExtend(projectResources, availabilityZoneResources, OpenstackConstant.RES_TYPE_AVAILABILITY_ZONE);
        fillProjectExtend(projectResources, networkResources, OpenstackConstant.PROJECT_NETWORK);
        HashMap<String, Integer> projectHostCountMap = new HashMap<>();
        fillCloudServerProperties(cloudServerResources, cloudServerDiskResources, projectMap, projectHostCountMap);
        fillProjectCount(projectResources, projectHostCountMap);
        return cloudServerResources;
    }

    private void fillProjectExtend(List<ProtectedResource> projectResources, List<ProtectedResource> resources,
        String key) {
        Map<String, List<Map<String, String>>> resourceMap = resources.stream()
            .collect(Collectors.groupingBy(ResourceBase::getParentUuid,
                Collectors.mapping(networkResource -> networkResource.getExtendInfo(), Collectors.toList())));
        projectResources.forEach(protectedResource -> {
            protectedResource.getExtendInfo()
                .put(key,
                    JSONObject.stringify(resourceMap.getOrDefault(protectedResource.getUuid(), Lists.newArrayList())));
        });
    }

    private void fillDomainProperties(List<ProtectedResource> domainResources,
        List<ProtectedResource> visibleDomainResources, ProtectedEnvironment env) {
        Set<String> validUuids = visibleDomainResources.stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toSet());
        domainResources.forEach(domain -> setDomainProperties(env, validUuids, domain));
    }

    private void setDomainProperties(ProtectedEnvironment env, Set<String> validUuids, ProtectedResource domain) {
        if (Objects.equals(domain.getUuid(), OpenstackConstant.DEFAULT_DOMAIN_ID)) {
            domain.setUuid(env.getExtendInfo().get(OpenstackConstant.SERVICE_ID_KEY));
        }
        domain.setParentUuid(env.getUuid());
        domain.setParentName(env.getName());
        domain.setPath(domain.getName());
        if (!validUuids.contains(domain.getUuid())) {
            domain.getExtendInfo().put(OpenstackConstant.VISIBLE, OpenstackDomainVisibleEnum.INVISIBLE.getCode());
        }
        if (StringUtils.isBlank(domain.getUserId()) && StringUtils.isNotBlank(env.getUserId())) {
            domain.setUserId(env.getUserId());
            domain.setAuthorizedUser(env.getAuthorizedUser());
        }
    }

    private void fillProjectCount(List<ProtectedResource> projectResources, Map<String, Integer> projectHostCountMap) {
        projectResources.forEach(project -> {
            Map<String, String> extendInfo = Optional.ofNullable(project.getExtendInfo()).orElse(new HashMap<>());
            Integer cloudHostCount = projectHostCountMap.getOrDefault(project.getUuid(), LegoNumberConstant.ZERO);
            extendInfo.put(OpenstackConstant.CLOUD_SERVER_COUNT, cloudHostCount.toString());
            project.setExtendInfo(extendInfo);
        });
    }

    private void fillCloudServerProperties(List<ProtectedResource> cloudServerResources,
        List<ProtectedResource> cloudServerDiskResources, Map<String, ProtectedResource> projectMap,
        HashMap<String, Integer> projectHostCountMap) {
        Map<String, List<VolInfo>> serverDisksMap = cloudServerDiskResources.stream().collect(Collectors.groupingBy(
            ResourceBase::getParentUuid, Collectors.mapping(
                resource ->
                    JSON.toJavaObject(JSON.parseObject(JSON.toJSONString(resource.getExtendInfo())), VolInfo.class),
                Collectors.toList())));
        cloudServerResources.removeIf(resource -> Objects.equals(
            resource.getExtendInfo().get(OpenstackConstant.SERVER_STATUS), OpenstackConstant.ERROR_STATUS));
        cloudServerResources.forEach(cloudServerResource -> {
            ProtectedResource project = projectMap.get(cloudServerResource.getParentUuid());
            cloudServerResource.setPath(project.getPath() + File.separator + cloudServerResource.getName());
            cloudServerResource.setUserId(project.getUserId());
            cloudServerResource.setAuthorizedUser(project.getAuthorizedUser());
            List<VolInfo> volInfos = serverDisksMap.getOrDefault(cloudServerResource.getUuid(), Lists.newArrayList());
            volInfos = volInfos.stream()
                .filter(volInfo -> Objects.equals(volInfo.getShareable(), Boolean.FALSE.toString())
                    && !Objects.equals(volInfo.getFullClone(), OpenstackConstant.CLONE_VOL))
                .collect(Collectors.toList());
            cloudServerResource.getExtendInfo().put(OpenstackConstant.VOLUME_INFO_KEY, JSONObject.stringify(volInfos));
            // 添加项目下云主机数量
            projectHostCountMap.merge(project.getUuid(), LegoNumberConstant.ONE, Integer::sum);
        });
    }

    private void fillProjectProperties(List<ProtectedResource> projectResources, ProtectedEnvironment env,
        Map<String, Integer> domainProjectCountMap) {
        projectResources.forEach(projectResource -> {
            projectResource.setPath(projectResource.getParentName() + File.separator + projectResource.getName());
            projectResource.setUserId(env.getUserId());
            projectResource.setAuthorizedUser(env.getAuthorizedUser());
            Map<String, String> extendInfo = projectResource.getExtendInfo();
            String domainId = projectResource.getParentUuid();
            if (domainId.equals(OpenstackConstant.DEFAULT_DOMAIN_ID)) {
                String serviceId = env.getExtendInfoByKey(OpenstackConstant.SERVICE_ID_KEY);
                projectResource.setParentUuid(serviceId);
            }
            extendInfo.put(OpenstackConstant.DOMAIN_ID_KEY, convertDefaultDomainId(env, domainId));
            extendInfo.put(OpenstackConstant.DOMAIN_NAME_KEY, projectResource.getParentName());
            domainProjectCountMap.merge(projectResource.getParentUuid(), LegoNumberConstant.ONE, Integer::sum);
        });
    }

    private List<ProtectedResource> doScanResources(Endpoint endpoint, List<Application> applications,
        String condition, ProtectedEnvironment env) {
        int page = LegoNumberConstant.ZERO;
        int size = LegoNumberConstant.FIVE * LegoNumberConstant.HUNDRED;
        ListResourceV2Req request = new ListResourceV2Req();
        request.setPageSize(size);
        request.setPageNo(page);
        request.setAppEnv(BeanTools.copy(env, AppEnv::new));
        request.setApplications(applications);
        Map<String, String> conditions = new HashMap<>();
        conditions.put(OpenstackConstant.RESOURCE_TYPE_KEY, condition);
        request.setConditions(JSON.toJSONString(conditions));
        PageListResponse<ProtectedResource> response;
        List<ProtectedResource> scanResources = Lists.newArrayList();
        do {
            request.setPageNo(page);
            response = agentService.getDetailPageList(env.getSubType(), endpoint.getIp(), endpoint.getPort(),
                request);
            List<ProtectedResource> protectedResources = response.getRecords();
            if (CollectionUtils.isEmpty(protectedResources)) {
                break;
            }
            page++;
            scanResources.addAll(protectedResources);
            // 将最后一条记录id作为下次查询的标记
            if (condition.equals(OpenstackConstant.RES_TYPE_SERVER)) {
                conditions.put(OpenstackConstant.SCAN_MARKER_KEY,
                    protectedResources.get(protectedResources.size() - 1).getUuid());
                request.setConditions(JSON.toJSONString(conditions));
            }
        } while (response.getRecords().size() == size);
        // 根据uuid去重
        scanResources = scanResources.stream()
            .peek(resource -> resource.setRootUuid(env.getUuid()))
            .collect(Collectors.collectingAndThen(Collectors.toCollection(
                () -> new TreeSet<>(Comparator.comparing(ResourceBase::getUuid))), Lists::newArrayList));
        log.info("scan resources success, condition:{}, total:{}", condition, scanResources.size());
        return scanResources;
    }

    private List<ProtectedEnvironment> getAllAgents(ProtectedEnvironment env) {
        Map<ProtectedResource, List<ProtectedEnvironment>> map =
            envRetrievalsService.collectConnectableResources(env.getUuid());
        return map.values().stream().flatMap(List::stream).collect(Collectors.toList());
    }

    private Endpoint getAgentEndpoint(String agentId) {
        Optional<ProtectedResource> optResource = resourceService.getResourceById(agentId);
        return optResource.flatMap(OptionalUtil.match(ProtectedEnvironment.class))
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "Agent is empty"));
    }

    private List<Application> convertToApps(List<ProtectedResource> parentResources,
        Map<String, Authentication> domainAuthMap, ProtectedEnvironment environment) {
        return parentResources.stream()
            .map(resource -> BeanTools.copy(resource, Application::new))
            .peek(app -> {
                // 扫描project时需要转换默认domain的id，扫描project和虚拟机时需要domain的认证信息
                app.setUuid(convertDefaultDomainId(environment, app.getUuid()));
                app.setAuth(domainAuthMap.getOrDefault(app.getParentUuid(), app.getAuth()));
            })
            .collect(Collectors.toList());
    }

    private String convertDefaultDomainId(ProtectedEnvironment environment, String domainId) {
        boolean isDefault = domainId.equals(environment.getExtendInfo().get(OpenstackConstant.SERVICE_ID_KEY));
        return isDefault ? OpenstackConstant.DEFAULT_DOMAIN_ID : domainId;
    }

    private List<ProtectedResource> getVisibleDomainResource(ProtectedEnvironment env) {
        int page = LegoNumberConstant.ZERO;
        int size = LegoNumberConstant.THOUSAND;
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("subType", ResourceSubTypeEnum.OPENSTACK_DOMAIN.getType());
        conditions.put("parentUuid", env.getUuid());
        conditions.put(OpenstackConstant.VISIBLE, OpenstackDomainVisibleEnum.VISIBLE.getCode());
        PageListResponse<ProtectedResource> domainResResponse = resourceService.query(page, size, conditions);
        return domainResResponse.getRecords();
    }
}
