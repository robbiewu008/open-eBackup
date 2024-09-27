package openbackup.oracle.provider;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * Oracle集群环境注册provider
 *
 * @author c30038333
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-27
 */
@Component
@Slf4j
public class OracleClusterProvider extends DatabaseEnvironmentProvider {
    private static final String AGENTS_INFO = "agentsInfo";

    private final OracleBaseService oracleBaseService;
    private final ResourceService resourceService;
    private final OracleClusterDatabaseProvider databaseProvider;

    /**
     * DatabaseResourceProvider
     *
     * @param providerManager     provider manager
     * @param pluginConfigManager provider config manager
     * @param oracleBaseService oracleBaseService
     * @param resourceService resourceService
     * @param databaseProvider databaseProvider
     */
    public OracleClusterProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
            OracleBaseService oracleBaseService, ResourceService resourceService,
            OracleClusterDatabaseProvider databaseProvider) {
        super(providerManager, pluginConfigManager);
        this.oracleBaseService = oracleBaseService;
        this.resourceService = resourceService;
        this.databaseProvider = databaseProvider;
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("Oracle cluster check start, name: {}, uuid: {}", environment.getName(), environment.getUuid());
        // 检查节点数量是否大于1
        checkAgentNumber(environment);

        // 检查agent是否离线，任意agent离线无法注册
        checkAgent(environment);

        // 检查集群是否重复注册
        checkRepeats(environment);

        // 设置uuid
        fillEnvUuid(environment);

        // 设置集群在线状态
        updateLinkStatus(environment);

        // 设置endpoint, 否则集群资源授权会报错！
        updateEndpoint(environment);

        // 设置osType,适配windows操作系统
        fillOsType(environment);
        log.info("Oracle cluster check finished, name: {}, uuid: {}", environment.getName(), environment.getUuid());
    }

    private void fillOsType(ProtectedEnvironment environment) {
        String agentId = environment.getDependencies().get(DatabaseConstants.AGENTS).stream()
                .map(ResourceBase::getUuid).findFirst().get();
        ProtectedEnvironment agent = oracleBaseService.getEnvironmentById(agentId);
        environment.setOsType(agent.getOsType());
    }

    private void checkAgentNumber(ProtectedEnvironment environment) {
        List<ProtectedResource> agents = environment.getDependencies().get(DatabaseConstants.AGENTS);
        if (VerifyUtil.isEmpty(agents)) {
            throw new LegoCheckedException(DatabaseErrorCode.CLUSTER_NODE_NUMBER_ERROR, "cluster node number error.");
        }
    }

    private void checkAgent(ProtectedEnvironment environment) {
        List<String> uuids = environment.getDependencies().get(DatabaseConstants.AGENTS).stream()
                .map(ResourceBase::getUuid).collect(Collectors.toList());
        uuids.forEach(this::checkSingleAgent);
    }


    private void checkSingleAgent(String agentId) {
        ProtectedEnvironment agent = oracleBaseService.getEnvironmentById(agentId);
        if (LinkStatusEnum.OFFLINE.getStatus().toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(agent))) {
            log.error("Host is offline, id: {}", agentId);
            throw new DataProtectionAccessException(CommonErrorCode.HOST_OFFLINE, new String[]{},
                    "Protected environment is offLine!");
        }
        if (!ResourceSubTypeEnum.U_BACKUP_AGENT.equalsSubType(agent.getSubType())) {
            log.error("Host subType is not supported, id: {}, subType: {}", agentId, agent.getSubType());
            throw new DataProtectionAccessException(CommonErrorCode.ERR_PARAM, new String[]{},
                    "Host subType is not supported!");
        }
    }

    private void checkRepeats(ProtectedEnvironment environment) {
        String agentsId = environment.getDependencies().get(DatabaseConstants.AGENTS).stream()
                .map(ResourceBase::getUuid).sorted().collect(Collectors.joining(","));
        String agentsInfo = UUID.nameUUIDFromBytes(agentsId.getBytes(Charset.defaultCharset())).toString();
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(AGENTS_INFO, agentsInfo);
        PageListResponse<ProtectedResource> clusters = resourceService.query(0, 1, conditions);
        if (!VerifyUtil.isEmpty(clusters.getRecords())
                && !clusters.getRecords().get(0).getUuid().equals(environment.getUuid())) {
            log.error("oracle cluster has been registered. name: {}, agentsId: {}", environment.getName(), agentsId);
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED, "oracle cluster was registered");
        }
        environment.setExtendInfoByKey(AGENTS_INFO, agentsInfo);
    }

    private void fillEnvUuid(ProtectedEnvironment environment) {
        environment.setUuid(Optional.ofNullable(environment.getUuid()).orElse(UUIDGenerator.getUUID()));
        log.info("Env set uuid succeed, name: {}, id: {}", environment.getName(), environment.getUuid());
    }

    private void updateLinkStatus(ProtectedEnvironment environment) {
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    private void updateEndpoint(ProtectedEnvironment cluster) {
        String agentId = cluster.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid();
        cluster.setEndpoint(oracleBaseService.getEnvironmentById(agentId).getEndpoint());
    }

    @Override
    public void validate(ProtectedEnvironment environment) {
        doDatabaseHealthCheck(environment);
        doClusterHealthCheck(environment);
        updateOsTypeToDb(environment);
    }

    private void updateOsTypeToDb(ProtectedEnvironment environment) {
        if (!VerifyUtil.isEmpty(environment.getOsType())) {
            return;
        }
        String agentId = environment.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(ResourceBase::getUuid)
            .findFirst()
            .orElse(StringUtils.EMPTY);
        ProtectedEnvironment agent = oracleBaseService.getEnvironmentById(agentId);
        ProtectedEnvironment env = new ProtectedEnvironment();
        env.setUuid(environment.getUuid());
        env.setOsType(agent.getOsType());
        resourceService.updateSourceDirectly(Collections.singletonList(env));
    }

    private void doDatabaseHealthCheck(ProtectedEnvironment cluster) {
        List<ProtectedResource> databases = getClusterDatabase(cluster);
        databases.forEach(databaseProvider::healthCheck);
        log.info("Oracle cluster database health check finish, database ids: {}",
                JsonUtil.json(databases.stream().map(ProtectedResource::getUuid).collect(Collectors.toList())));
    }

    // 直接根据agent状态来判断集群环境状态, 任意节点在线，集群在线
    private void doClusterHealthCheck(ProtectedEnvironment environment) {
        List<ProtectedResource> agents = environment.getDependencies().get(DatabaseConstants.AGENTS);
        for (ProtectedResource agent : agents) {
            if (agent instanceof ProtectedEnvironment
                    && LinkStatusEnum.ONLINE.getStatus().toString()
                    .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster((ProtectedEnvironment) agent))) {
                log.info("Oracle cluster health check success, name: {}, id: {}, agentId: {}",
                        environment.getName(), environment.getUuid(), agent.getUuid());
                return;
            }
        }
        log.error("Oracle cluster health check failed, name: {}, id: {}.",
                environment.getName(), environment.getUuid());
        throw new DataProtectionAccessException(CommonErrorCode.HOST_OFFLINE, new String[]{},
                "Oracle cluster is offLine!");
    }

    private List<ProtectedResource> getClusterDatabase(ProtectedEnvironment cluster) {
        PageListResponse<ProtectedResource> response;
        List<ProtectedResource> resources = new ArrayList<>();
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("subType", ResourceSubTypeEnum.ORACLE_CLUSTER.getType());
        conditions.put("parentUuid", cluster.getUuid());
        int pageNo = 0;
        do {
            response = resourceService.query(pageNo, IsmNumberConstant.HUNDRED, conditions);
            resources.addAll(response.getRecords());
            pageNo++;
        } while (response.getRecords().size() >= IsmNumberConstant.HUNDRED);
        return resources;
    }

    @Override
    public void remove(ProtectedEnvironment environment) {
        if (!VerifyUtil.isEmpty(getClusterDatabase(environment))) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_BE_DEPENDED_BY_OTHERS,
                    "oracle cluster delete failed");
        }
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.ORACLE_CLUSTER_ENV.equalsSubType(resourceSubType);
    }
}
