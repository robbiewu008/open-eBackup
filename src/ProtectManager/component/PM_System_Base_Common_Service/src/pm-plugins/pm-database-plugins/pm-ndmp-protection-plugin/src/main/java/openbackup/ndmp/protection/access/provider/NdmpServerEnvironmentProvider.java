/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.ndmp.protection.access.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.ndmp.protection.access.common.NdmpCommon;
import openbackup.ndmp.protection.access.constant.NdmpConstant;
import openbackup.ndmp.protection.access.service.NdmpService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;
import org.springframework.transaction.annotation.Transactional;

import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Optional;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * NDMP-server 资源接入
 *
 * @author t30021437
 * @since 2023-05-06
 */
@Slf4j
@Component
public class NdmpServerEnvironmentProvider extends DatabaseEnvironmentProvider {
    private final NdmpService ndmpService;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * DatabaseResourceProvider
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     * @param ndmpService ndmpService
     * @param agentUnifiedService agentUnifiedService
     */
    public NdmpServerEnvironmentProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        NdmpService ndmpService, AgentUnifiedService agentUnifiedService) {
        super(providerManager, pluginConfigManager);
        this.ndmpService = ndmpService;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.NDMP_SERVER.getType().equals(object);
    }

    /**
     * 资源校验
     *
     * @param environment 受保护环境
     */
    @Override
    @Transactional(rollbackFor = Exception.class)
    public void register(ProtectedEnvironment environment) {
        log.info("start to check ndmp-server environment");

        // 检查连通性 checkApplication
        log.info("ndmp-server ndmpServerGap {}", environment.getExtendInfoByKey(NdmpConstant.EXTEND_INFO_KEY_NDMP_GAP));
        if (!VerifyUtil.isEmpty(environment.getUuid())) {
            List<ProtectedEnvironment> existServers = getExistPrv(environment.getUuid());
            if (existServers.isEmpty()) {
                throw new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST, "no exist NDMP-server resources");
            }
            if (VerifyUtil.isEmpty(environment.getAuth().getAuthPwd())) {
                environment.setAuth(existServers.get(NdmpConstant.INT_ZERO).getAuth());
            }
        }
        List<ProtectedResource> protectedEnvironments = ndmpService.getAvailableAgents(environment);
        if (VerifyUtil.isEmpty(protectedEnvironments)) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "interAgent is not available");
        }

        if (VerifyUtil.isEmpty(environment.getUuid())) {
            generateUniqueUuid(environment);
            log.info("success to generate uuid");
        }
        environment.setPath(environment.getEndpoint());
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("finish the NDMP-server check");
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        log.info("Start to scan ndmp-server environment,environment_uuid:{}", environment.getUuid());
        List<ProtectedResource> protectedEnvironments = ndmpService.getAvailableAgents(environment);
        ProtectedEnvironment protectedEnvironment = protectedEnvironments.stream()
            .filter(resource -> resource instanceof ProtectedEnvironment)
            .map(resource -> (ProtectedEnvironment) resource)
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "No agent online"));
        log.info("Start to scan ndmp-server environment,agent_uuid:{}", protectedEnvironment.getUuid());
        ndmpService.checkApplication(environment, protectedEnvironment);
        int count = NdmpConstant.INT_ZERO;
        PageListResponse<ProtectedResource> response;
        List<ProtectedResource> scanResources = Lists.newArrayList();
        do {
            response = agentUnifiedService.getDetailPageList(ResourceSubTypeEnum.NDMP.getType(),
                protectedEnvironment.getEndpoint(), protectedEnvironment.getPort(),
                generateListResourceV2Req(count++, NdmpConstant.QUERY_SIZE, environment, protectedEnvironment));
            if (!response.getRecords().isEmpty()) {
                List<ProtectedResource> protectedResources = response.getRecords();
                for (ProtectedResource protectedResource : protectedResources) {
                    NdmpCommon.setNdmpNames(protectedResource);
                    protectedResource.setParentName(environment.getName());
                    protectedResource.setParentUuid(environment.getUuid());
                    protectedResource.setRootUuid(environment.getUuid());
                    protectedResource.setUuid(createResourceUuid(environment.getUuid(), protectedResource));
                }
                scanResources.addAll(protectedResources);
            }
        } while (response.getRecords().size() == NdmpConstant.QUERY_SIZE);

        log.info("the scan resource size {}", scanResources.size());
        return scanResources;
    }

    private String createResourceUuid(String environmentUuid, ProtectedResource protectedResource) {
        String protectedResourceUuid = environmentUuid + ResourceSubTypeEnum.NDMP.getType()
            + protectedResource.getName();
        log.debug("File System:{} generated protectedResource UUID is:{}.", protectedResource.getName(),
            protectedResourceUuid);
        return UUID.nameUUIDFromBytes(protectedResourceUuid.getBytes(StandardCharsets.UTF_8)).toString();
    }

    private ListResourceV2Req generateListResourceV2Req(int page, int size, ProtectedEnvironment environment,
        ProtectedResource agentResource) {
        ListResourceV2Req listResourceV2Req = new ListResourceV2Req();
        environment.setSubType(ResourceSubTypeEnum.NDMP.getType());
        listResourceV2Req.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        listResourceV2Req.setPageSize(size);
        listResourceV2Req.setPageNo(page);
        agentResource.setSubType(ResourceSubTypeEnum.NDMP.getType());
        listResourceV2Req.setApplications(Lists.newArrayList(BeanTools.copy(agentResource, Application::new)));
        return listResourceV2Req;
    }

    private void generateUniqueUuid(ProtectedEnvironment environment) {
        String ndmpServerUuid = getUniqueUUID(environment.getEndpoint());
        environment.setUuid(ndmpServerUuid);
        environment.setRootUuid(ndmpServerUuid);
        environment.setParentUuid(ndmpServerUuid);
    }

    private String getUniqueUUID(String managerIp) {
        // 设置唯一UUID
        String envIdentity = managerIp + ResourceSubTypeEnum.NDMP.getType();
        return UUID.nameUUIDFromBytes(envIdentity.getBytes(Charset.defaultCharset())).toString();
    }

    private List<ProtectedEnvironment> getExistPrv(String uuid) {
        return ndmpService.getexistingNdmpresources(
            ImmutableMap.of("uuid", uuid, "type", ResourceTypeEnum.STORAGE_EQUIPMENT.getType(), "subType",
                ResourceSubTypeEnum.NDMP_SERVER.getType()))
            .stream()
            .filter(existingResource -> existingResource instanceof ProtectedEnvironment)
            .map(existingResource -> (ProtectedEnvironment) existingResource)
            .collect(Collectors.toList());
    }

    @Override
    public Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment) {
        log.info("The healthCheck of the NDMP-server, uuid: {},", environment.getUuid());
        List<ProtectedEnvironment> existServers = getExistPrv(environment.getUuid());
        if (existServers.isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST, "no exist NDMP-server resources");
        }
        List<ProtectedResource> protectedEnvironments = ndmpService.getAvailableAgents(environment);
        if (VerifyUtil.isEmpty(protectedEnvironments)) {
            return Optional.of(LinkStatusEnum.OFFLINE.getStatus().toString());
        }
        return Optional.of(LinkStatusEnum.ONLINE.getStatus().toString());
    }
}
