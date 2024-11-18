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
package openbackup.gaussdbdws.protection.access.provider;

import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import openbackup.gaussdbdws.protection.access.util.DwsTaskEnvironmentUtil;
import openbackup.gaussdbdws.protection.access.util.DwsValidator;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.ObjectUtils;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * GaussDbdws project资源相关接口的具体实现类
 * 实现了：扫描，健康状态检查，资源浏览，环境信息检查相关等接口
 *
 */
@Slf4j
@Component
public class GaussDBDWSProjectEnvironmentProvider extends DatabaseEnvironmentProvider {
    private final UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker;

    private final GaussDBBaseService gaussDBBaseService;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * GaussDBDWSClusterEnvironmentProvider
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     * @param clusterIntegrityChecker clusterIntegrityChecker
     * @param gaussDBBaseService gaussDBBaseService
     * @param agentUnifiedService agentUnifiedService
     */
    public GaussDBDWSProjectEnvironmentProvider(ProviderManager providerManager,
        PluginConfigManager pluginConfigManager, @Qualifier("unifiedClusterResourceIntegrityChecker")
        UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker, GaussDBBaseService gaussDBBaseService,
        AgentUnifiedService agentUnifiedService) {
        super(providerManager, pluginConfigManager);
        this.clusterIntegrityChecker = clusterIntegrityChecker;
        this.gaussDBBaseService = gaussDBBaseService;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.GAUSSDB_DWS_PROJECT.getType().equals(object);
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        List<ProtectedResource> list = new ArrayList<>();
        List<ProtectedResource> envResourceList = gaussDBBaseService.getEnvResourceList(environment.getUuid(),
            ResourceSubTypeEnum.GAUSSDB_DWS.getType(), "autoscan");
        List<ProtectedResource> clusterResources = DwsTaskEnvironmentUtil.getAgentResourcesByKey(environment,
            DwsConstant.DWS_CLUSTER_AGENT);
        ProtectedEnvironment protectedEnvironment = gaussDBBaseService.getClusterUuid(clusterResources);
        ProtectedResource protectedResource = DwsTaskEnvironmentUtil.getProtectedResource(environment);
        protectedResource.setSubType(ResourceSubTypeEnum.GAUSSDB_DWS_PROJECT.getType());

        List<ProtectedResource> envList = new ArrayList<>();
        try {
            envList = getProtectResourceList(protectedEnvironment, environment);
        } catch (LegoCheckedException e) {
            log.error("scan project failed, uuid: {}", environment.getUuid());
            envResourceList.forEach(envResource -> {
                ProtectedEnvironment env = copyProtectedEnv(environment, envResource, "ture");
                env.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
                list.add(env);
            });
            return list;
        }
        list.addAll(getAllClusterResource(envList, envResourceList, environment));
        return list;
    }

    private List<ProtectedEnvironment> getAllClusterResource(List<ProtectedResource> envList,
        List<ProtectedResource> envResourceList, ProtectedEnvironment environment) {
        List<ProtectedEnvironment> list = new ArrayList<>();
        envResourceList.forEach(envResource ->
            list.add(copyProtectedEnv(environment, envResource, "false"))
        );
        List<String> envUuid = gaussDBBaseService.getAllTableAndSchemaUuidList();
        Map<String, ProtectedEnvironment> listMap = list.stream()
            .collect(Collectors.toMap(ProtectedEnvironment::getUuid, Function.identity()));
        for (ProtectedResource envResource : envList) {
            if (ObjectUtils.isNotEmpty(listMap.get(envResource.getUuid()))) {
                continue;
            }
            if (!envUuid.contains(envResource.getUuid())) {
                continue;
            }
            list.add(copyProtectedEnv(environment, envResource, "ture"));
        }
        return list;
    }

    private ProtectedEnvironment copyProtectedEnv(ProtectedEnvironment environment, ProtectedResource envResource,
        String isDelete) {
        ProtectedEnvironment env = BeanTools.copy(envResource, ProtectedEnvironment::new);
        env.setLinkStatus(
            DwsConstant.DWS_CLUSTER_STATUS.get(env.getExtendInfoByKey(DwsConstant.EXTEND_INFO_KEY_STATE)));
        env.setVersion(env.getExtendInfoByKey(DwsConstant.EXTEND_INFO_KEY_VERSION));
        env.setEndpoint(environment.getEndpoint());
        env.setPath(environment.getPath());
        env.setRootUuid(env.getUuid());
        env.setParentName(env.getName());
        env.setParentUuid(env.getUuid());
        env.setExtendInfoByKey(DwsConstant.IS_PROJECT_SCAN, "true");
        env.setExtendInfoByKey(DwsConstant.IS_DELETE, isDelete);
        env.setExtendInfoByKey(DwsConstant.IS_PROJECT, "true");
        return env;
    }

    private List<ProtectedResource> getProtectResourceList(ProtectedEnvironment env, ProtectedEnvironment environment) {
        List<ProtectedResource> protectedResources = new ArrayList<>();
        BrowseEnvironmentResourceConditions environmentConditions = new BrowseEnvironmentResourceConditions();
        environmentConditions.setResourceType(ResourceSubTypeEnum.GAUSSDB_DWS_PROJECT.getType());
        environmentConditions.setParentId("/");
        environmentConditions.setPageNo(0);
        environmentConditions.setPageSize(DwsConstant.QUERY_QUANTITY_SPECIFICATIONS);
        PageListResponse<ProtectedResource> detailPageList = agentUnifiedService.getDetailPageListNoRetry(
            environmentConditions.getResourceType(), env.getEndpoint(), env.getPort(),
            gaussDBBaseService.getListResourceReq(env, environment, environmentConditions), false);
        int count = detailPageList.getTotalCount() / DwsConstant.QUERY_QUANTITY_SPECIFICATIONS;
        for (int size = 0; size <= count; size++) {
            environmentConditions.setPageNo(size);
            PageListResponse<ProtectedResource> environments = agentUnifiedService.getDetailPageListNoRetry(
                environmentConditions.getResourceType(), env.getEndpoint(), env.getPort(),
                gaussDBBaseService.getListResourceReq(env, environment, environmentConditions), false);
            protectedResources.addAll(environments.getRecords());
        }
        return protectedResources;
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("dws start project update name: {}, uuid: {}", environment.getName(), environment.getUuid());
        // 判断name是否为空
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(environment.getName());
        //  终端节点是否为空;
        DwsValidator.checkDwsValue(
            Optional.ofNullable(environment.getExtendInfo()).orElse(new HashMap<>()).get(DwsConstant.TERMINAL_NODE));
        // IAM用户所属账号
        DwsValidator.checkDwsValue(
            Optional.ofNullable(environment.getExtendInfo()).orElse(new HashMap<>()).get(DwsConstant.IAM_USER_ACCOUNT));
        //  用户名  为空 拦截报错;
        DwsValidator.checkDwsValue(
            Optional.ofNullable(environment.getAuth()).orElse(new Authentication()).getAuthKey());
        //  密码为空 拦截报错;
        DwsValidator.checkDwsValue(
            Optional.ofNullable(environment.getAuth()).orElse(new Authentication()).getAuthPwd());

        // 获取已注册成功的DWS项目信息
        List<ProtectedEnvironment> existingEnvironments = gaussDBBaseService.getExistingDwsResources(
            ResourceSubTypeEnum.GAUSSDB_DWS_PROJECT.getType());
        // 判断 是否存在重复的集群节点或者代理主机
        checkDuplicateClusterAndHost(existingEnvironments, environment);

        ProtectedResource protectedResource = DwsTaskEnvironmentUtil.getProtectedResource(environment);

        // 通过不同集群节点调用完成资源查询, CentralCoordinator IP 是否完全一致;
        List<ProtectedResource> clusterResources = DwsTaskEnvironmentUtil.getAgentResourcesByKey(environment,
            DwsConstant.DWS_CLUSTER_AGENT);
        //  集群节点 为空 拦截报错;
        DwsValidator.checkDwsValue((clusterResources.size() == 0) ? "" : DwsConstant.DWS_CLUSTER_AGENT);
        gaussDBBaseService.checkConnention(protectedResource);
        // 规格检查 是否大于8个
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            DwsValidator.checkDwsCount(existingEnvironments);
        }
        AppEnvResponse appEnv = getCentralCoordinator(clusterResources, protectedResource);
        environment.setUuid(appEnv.getExtendInfo().get(DwsConstant.PROJECT_ID));
        DwsValidator.checkDwsResourceExistById(existingEnvironments, environment.getUuid());
        // 添加对象信息位置
        environment.setPath(
            Optional.ofNullable(environment.getExtendInfo()).orElse(new HashMap<>()).get(DwsConstant.TERMINAL_NODE));
        environment.setEndpoint(
            Optional.ofNullable(environment.getExtendInfo()).orElse(new HashMap<>()).get(DwsConstant.TERMINAL_NODE));
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        // 设置为集群;
        environment.setCluster(true);

        log.info("dws project update name: {}, uuid: {}", environment.getName(), environment.getUuid());
    }

    private void checkDuplicateClusterAndHost(List<ProtectedEnvironment> existingEnvironments,
        ProtectedEnvironment protectedResource) {
        List<String> clusterUuidList = Optional.ofNullable(
            protectedResource.getDependencies().get(DwsConstant.DWS_CLUSTER_AGENT))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "name is not exist"))
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        List<ProtectedEnvironment> existingEnvironmentList = existingEnvironments.stream()
            .map(ProtectedEnvironment::getUuid)
            .filter(clusterUuid -> !clusterUuid.equals(protectedResource.getUuid()))
            .map(gaussDBBaseService::getEnvironmentById)
            .collect(Collectors.toList());
        DwsValidator.checkDwsExistSameClusterOrHost(existingEnvironmentList, clusterUuidList);
    }

    private AppEnvResponse getCentralCoordinator(List<ProtectedResource> clusterResources,
        ProtectedResource protectedResource) {
        ProtectedEnvironment protectedEnvironment = gaussDBBaseService.getClusterUuid(clusterResources);
        protectedResource.setEnvironment(protectedEnvironment);
        CheckResult<AppEnvResponse> appEnvResponseCheckResult = clusterIntegrityChecker.generateCheckResult(
            protectedResource);
        AppEnvResponse appEnv = appEnvResponseCheckResult.getData();
        if (ObjectUtils.isEmpty(appEnvResponseCheckResult.getData())
            || appEnvResponseCheckResult.getData().getExtendInfo() == null) {
            log.error("The GaussDB-DWS project nodes query failed.");
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                "The GaussDB-DWS project nodes query failed.");
        }
        return appEnv;
    }

    /**
     * 受保护环境健康状态检查, 返回连接状态
     *
     * @param environment 受保护环境
     */
    @Override
    public void validate(ProtectedEnvironment environment) {
        log.info("dws check health project update name: {}, uuid: {}", environment.getName(), environment.getUuid());
        gaussDBBaseService.checkConnention(DwsTaskEnvironmentUtil.getProtectedResource(environment));
        log.info("dws check health project end name: {}, uuid: {}", environment.getName(), environment.getUuid());
    }
}