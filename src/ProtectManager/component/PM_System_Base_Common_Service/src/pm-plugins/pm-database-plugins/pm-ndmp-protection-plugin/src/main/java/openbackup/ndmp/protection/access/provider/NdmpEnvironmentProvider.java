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
package openbackup.ndmp.protection.access.provider;

import com.huawei.oceanprotect.repository.service.LocalStorageService;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.ndmp.protection.access.constant.NdmpConstant;
import openbackup.ndmp.protection.access.service.NdmpService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.nio.charset.Charset;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * ndmp 资源接入
 *
 */
@Slf4j
@Component
public class NdmpEnvironmentProvider extends DatabaseEnvironmentProvider {
    private final NdmpService ndmpService;

    private final LocalStorageService localStorageService;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * DatabaseResourceProvider
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     * @param ndmpService ndmpService
     * @param localStorageService localStorageService
     * @param agentUnifiedService agentUnifiedService
     */
    public NdmpEnvironmentProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        NdmpService ndmpService, LocalStorageService localStorageService, AgentUnifiedService agentUnifiedService) {
        super(providerManager, pluginConfigManager);
        this.ndmpService = ndmpService;
        this.localStorageService = localStorageService;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.NDMP.getType().equals(object);
    }

    /**
     * 资源校验
     *
     * @param environment 受保护环境
     */
    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("start to check NDMP environment");
        log.info("start to query NDMP-server");
        List<ProtectedEnvironment> existingServerEnvironments = getExistPrv(ResourceSubTypeEnum.NDMP_SERVER,
            new HashMap<>());
        if (existingServerEnvironments.isEmpty() || !StringUtils.equals(NdmpConstant.STATUS_OPEN,
            existingServerEnvironments.get(NdmpConstant.INT_ZERO)
                .getExtendInfoByKey(NdmpConstant.EXTEND_INFO_KEY_NDMP_GAP))) {
            throw new LegoCheckedException(CommonErrorCode.TARGET_CLUSTER_AUTH_FAILED, "server auth failed");
        }
        buildPrvExtension(environment, existingServerEnvironments);
        log.info("NDMP-SERVER size is {} and auth success", existingServerEnvironments.size());

        // 检查连通性 checkApplication
        List<ProtectedResource> agentPrvs = ndmpService.getOneAgentHealthCheck(environment);

        if (VerifyUtil.isEmpty(agentPrvs)) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "interAgent is not available");
        }
        log.info("select {} interAgent is check success", agentPrvs.size());

        // 获取已注册成功的Ndmp资源信息
        List<ProtectedEnvironment> existingEnvironments = getExistPrv(ResourceSubTypeEnum.NDMP, new HashMap<>());

        // 去框架查询agent信息，不报错表明环境信息正常
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            log.info("start to generate uuid");

            // 校验注册集群是否重复并设置uuid
            generateUniqueUuid(environment, existingEnvironments);
        }
        log.info("start to endpoint");

        // 检查通过后，添加数据到environment中，由框架负责持久化
        environment.setEndpoint(agentPrvs.get(NdmpConstant.INT_ZERO).getEndpoint());
        environment.setPath(agentPrvs.get(NdmpConstant.INT_ZERO).getEndpoint());
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("finish the check");
    }

    private void buildPrvExtension(ProtectedEnvironment environment, List<ProtectedEnvironment> existServerPrv) {
        String ndmpSrc = existServerPrv.get(NdmpConstant.INT_ZERO).getExtendInfoByKey(NdmpConstant.NDMP_SRC);
        environment.setExtendInfoByKey(NdmpConstant.NDMP_SRC, ndmpSrc);
        environment.setExtendInfoByKey(NdmpConstant.VERSION,
            existServerPrv.get(NdmpConstant.INT_ZERO).getExtendInfoByKey(NdmpConstant.VERSION));
        environment.setExtendInfoByKey(NdmpConstant.EXTEND_INFO_KEY_NDMP_GAP,
            existServerPrv.get(NdmpConstant.INT_ZERO).getExtendInfoByKey(NdmpConstant.EXTEND_INFO_KEY_NDMP_GAP));
        Authentication auth = environment.getAuth();
        Map<String, String> extension = new HashMap<>();
        extension.put(NdmpConstant.AUTH_KEY, existServerPrv.get(NdmpConstant.INT_ZERO).getAuth().getAuthKey());
        extension.put(NdmpConstant.AUTH_PASS, existServerPrv.get(NdmpConstant.INT_ZERO).getAuth().getAuthPwd());
        auth.setExtendInfo(extension);
    }

    private List<ProtectedEnvironment> getExistPrv(ResourceSubTypeEnum subTypeEnum,
        Map<String, Object> filterConditions) {
        return ndmpService.getexistingNdmpresources(subTypeEnum.getType(), filterConditions)
            .stream()
            .filter(existingResource -> existingResource instanceof ProtectedEnvironment)
            .map(existingResource -> (ProtectedEnvironment) existingResource)
            .collect(Collectors.toList());
    }

    private void generateUniqueUuid(ProtectedEnvironment environment, List<ProtectedEnvironment> existingEnvironments) {
        String ndmpUuid = UUID.nameUUIDFromBytes(
            (environment.getName() + environment.getSubType() + localStorageService.getStorageInfo().getEsn()).getBytes(
                Charset.defaultCharset())).toString();
        environment.setUuid(ndmpUuid);
        environment.setRootUuid(ndmpUuid);
        environment.setParentUuid(ndmpUuid);
        boolean isNameOrUuidDuplicate = existingEnvironments.stream()
            .anyMatch(
                existEnv -> ndmpUuid.equals(existEnv.getUuid()) || environment.getName().equals(existEnv.getName()));
        if (isNameOrUuidDuplicate) {
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED, "register is duplicate.");
        }
    }

    @Override
    public Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment) {
        try {
            this.register(environment);
        } catch (LegoCheckedException exception) {
            log.error("query resources fail", ExceptionUtil.getErrorMessage(exception));
            return Optional.of(LinkStatusEnum.OFFLINE.getStatus().toString());
        }
        return Optional.of(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        List<ProtectedResource> agentResources = ndmpService.getOneAgentHealthCheck(environment);
        if (VerifyUtil.isEmpty(agentResources)) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "interAgent is not available");
        }

        ProtectedEnvironment protectedEnvironment = agentResources.stream()
            .filter(resource -> resource instanceof ProtectedEnvironment)
            .map(resource -> (ProtectedEnvironment) resource)
            .findFirst()
            .get();
        int count = NdmpConstant.INT_ZERO;
        PageListResponse<ProtectedResource> response;
        List<ProtectedResource> scanResources = Lists.newArrayList();
        do {
            response = agentUnifiedService.getDetailPageList(ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType(),
                protectedEnvironment.getEndpoint(), protectedEnvironment.getPort(),
                generateListResourceV2Req(count++, NdmpConstant.QUERY_SIZE, environment, protectedEnvironment));
            if (!response.getRecords().isEmpty()) {
                List<ProtectedResource> protectedResources = response.getRecords();
                scanResources.addAll(protectedResources);
            }
        } while (response.getRecords().size() == NdmpConstant.QUERY_SIZE);

        log.info("the scan resource size {}", scanResources.size());
        return scanResources;
    }

    private ListResourceV2Req generateListResourceV2Req(int page, int size, ProtectedEnvironment environment,
        ProtectedResource agentResource) {
        ListResourceV2Req listResourceV2Req = new ListResourceV2Req();
        environment.setSubType(ResourceSubTypeEnum.NDMP_BACKUPSET.getType());
        listResourceV2Req.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        listResourceV2Req.setPageSize(size);
        listResourceV2Req.setPageNo(page);
        agentResource.setSubType(ResourceSubTypeEnum.NDMP_BACKUPSET.getType());
        listResourceV2Req.setApplications(Lists.newArrayList(BeanTools.copy(agentResource, Application::new)));
        return listResourceV2Req;
    }

    @Override
    public PageListResponse<ProtectedResource> browse(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        List<ProtectedResource> agentResources = ndmpService.getOneAgentHealthCheck(environment);
        if (VerifyUtil.isEmpty(agentResources)) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "interAgent is not available");
        }
        ProtectedEnvironment protectedEnvironment = agentResources.stream()
            .filter(resource -> resource instanceof ProtectedEnvironment)
            .map(resource -> (ProtectedEnvironment) resource)
            .findFirst()
            .get();

        PageListResponse<ProtectedResource> detailPageList = agentUnifiedService.getDetailPageList(
            environmentConditions.getResourceType(), protectedEnvironment.getEndpoint(), protectedEnvironment.getPort(),
            getListResourceReq(protectedEnvironment, environment, environmentConditions));
        log.info("the query nas resources count is {}", detailPageList.getTotalCount());
        return detailPageList;
    }

    private ListResourceV2Req getListResourceReq(ProtectedEnvironment env, ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        AppEnv appEnv = BeanTools.copy(env, AppEnv::new);
        Application application = new Application();
        application.setAuth(environment.getAuth());
        application.setName(environmentConditions.getParentId());
        application.setType(environment.getType());
        application.setSubType(environmentConditions.getResourceType());
        application.setExtendInfo(environment.getExtendInfo());

        ListResourceV2Req req = new ListResourceV2Req();
        req.setPageNo(environmentConditions.getPageNo());
        req.setPageSize(environmentConditions.getPageSize());
        req.setAppEnv(appEnv);
        req.setConditions(environmentConditions.getConditions());
        req.setApplications(Lists.newArrayList(application));
        return req;
    }
}
