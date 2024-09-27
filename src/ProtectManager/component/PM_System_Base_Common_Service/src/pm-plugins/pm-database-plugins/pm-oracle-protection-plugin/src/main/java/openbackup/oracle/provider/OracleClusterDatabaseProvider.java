/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.oracle.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceReq;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.oracle.bo.OracleStorage;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.oracle.utils.StorageCheckUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.thread.ThreadPoolTool;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

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
 * OracleClusterDatabaseProvider
 *
 * @author l00570077
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-28
 */
@Slf4j
@Component
public class OracleClusterDatabaseProvider implements ResourceProvider {
    private static final String STORAGES = "storages";
    private static final String INSTALL_USERNAME = "installUsername";

    private static final String ACCESS_ORACLE_BASE = "accessOracleBase";

    private static final String ACCESS_ORACLE_HOME = "accessOracleHome";


    private final ResourceService resourceService;
    private final AgentUnifiedService agentUnifiedService;
    private final OracleBaseService oracleBaseService;

    /**
     * oracle集群注册provider构造函数
     *
     * @param resourceService resourceService
     * @param agentUnifiedService agentUnifiedService
     * @param oracleBaseService baseService
     */
    public OracleClusterDatabaseProvider(ResourceService resourceService, AgentUnifiedService agentUnifiedService,
            OracleBaseService oracleBaseService) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.oracleBaseService = oracleBaseService;
    }

    @Override
    public void beforeCreate(ProtectedResource clusterDatabase) {
        log.info("Oracle cluster database create check start, name: {}, parentUuid: {}.",
                clusterDatabase.getName(), clusterDatabase.getParentUuid());
        // 检查是否重复注册
        checkRegisterRepeats(clusterDatabase);

        preCheck(clusterDatabase);

        // 设置使用过的存储资源，用于删除资源上残留的保护组
        oracleBaseService.updateUsedStorageInfo(clusterDatabase);

        log.info("Oracle cluster database create check finish, name: {}, parentUuid: {}.",
                clusterDatabase.getName(), clusterDatabase.getParentUuid());
    }

    @Override
    public void beforeUpdate(ProtectedResource clusterDatabase) {
        log.info("Oracle cluster database update check start, name: {}, uuid: {}.",
                clusterDatabase.getName(), clusterDatabase.getParentUuid());

        preCheck(clusterDatabase);
        oracleBaseService.updateUsedStorageInfo(clusterDatabase);
        log.info("Oracle cluster database update check finish, name: {}, uuid: {}.",
                clusterDatabase.getName(), clusterDatabase.getParentUuid());
    }

    @Override
    public ResourceDeleteContext preHandleDelete(ProtectedResource resource) {
        log.info("start oracle cluster database delete pre check, resourceId:{}", resource.getUuid());
        if (!oracleBaseService.isAnonymizationDeletable(resource.getUuid())) {
            throw new LegoCheckedException(CommonErrorCode.ANONYMIZATION_JOB_IS_RUNNING, new String[] {},
                "resource has running anonymization job");
        }
        // 解除保护的时候，存储层备份需删除生产环境上保护组
        oracleBaseService.cleanDoradoEnv(resource);
        return ResourceDeleteContext.defaultValue();
    }

    @Override
    public List<ProtectedResource> scan(ProtectedResource resource) {
        log.info("Oracle cluster scan started, resource id :{}", resource.getUuid());
        List<ProtectedEnvironment> clusterHosts = oracleBaseService.getOracleClusterHosts(resource);
        List<ProtectedResource> instances = queryAllDatabaseInstances(resource, clusterHosts);
        updateClusterDatabase(resource, instances);
        log.info("Oracle cluster scan finished, resource id :{}", resource.getUuid());
        return Collections.singletonList(resource);
    }

    /**
     * 集群数据库连通性检查，任意节点检查成功即可返回成功
     *
     * @param database 集群数据库
     */
    @Override
    public void healthCheck(ProtectedResource database) {
        ThreadPoolTool.getPool().execute(() -> doHealthCheck(database));
    }

    private void doHealthCheck(ProtectedResource database) {
        log.info("Oracle cluster database health check start, name: {}, uuid: {}.",
                database.getName(), database.getUuid());
        try {
            List<ProtectedEnvironment> agents = oracleBaseService.getOracleClusterHosts(database);
            for (ProtectedEnvironment agent : agents) {
                AgentBaseDto response = agentUnifiedService.checkApplication(database, agent);
                if (OracleConstants.SUCCESS.equals(response.getErrorCode())) {
                    updateDatabaseStatusToDb(database.getUuid(), LinkStatusEnum.ONLINE);
                    return;
                }
            }
            updateDatabaseStatusToDb(database.getUuid(), LinkStatusEnum.OFFLINE);
        } catch (LegoCheckedException | LegoUncheckedException | DataProtectionAccessException e) {
            updateDatabaseStatusToDb(database.getUuid(), LinkStatusEnum.OFFLINE);
        }
    }

    private void preCheck(ProtectedResource clusterDatabase) {
        List<ProtectedEnvironment> clusterHosts = oracleBaseService.getOracleClusterHosts(clusterDatabase);

        // 检查添加的存储信息
        checkStorage(clusterDatabase, clusterHosts);

        // 检查连通性
        checkConnection(clusterDatabase, clusterHosts);

        // 从Agent查询实例信息
        List<ProtectedResource> instances = queryAllDatabaseInstances(clusterDatabase, clusterHosts);

        // 检查实例名称是否相同
        checkInstanceName(instances);

        // 更新集群数据库信息
        updateClusterDatabase(clusterDatabase, instances);
    }

    private void checkRegisterRepeats(ProtectedResource resource) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("parentUuid", resource.getParentUuid());
        conditions.put("name", resource.getName());
        conditions.put("subType", resource.getSubType());
        PageListResponse<ProtectedResource> resources = resourceService.query(0, 1, conditions);
        if (!VerifyUtil.isEmpty(resources.getRecords())) {
            log.error("Oracle cluster database has been registered, name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED,
                "This Oracle cluster has been registered");
        }
    }

    // 任意Agent检查失败，则连通性检查失败
    private void checkConnection(ProtectedResource clusterDatabase, List<ProtectedEnvironment> agents) {
        for (ProtectedEnvironment agent : agents) {
            AgentBaseDto response = agentUnifiedService.checkApplicationNoRetry(clusterDatabase, agent);
            checkConnectResult(response);
        }
        log.info("Check oracle cluster connection success, name: {}", clusterDatabase.getName());
    }

    private static void checkConnectResult(AgentBaseDto response) {
        if (!OracleConstants.SUCCESS.equals(response.getErrorCode())) {
            ActionResult actionResult = JSONObject.toBean(response.getErrorMessage(), ActionResult.class);
            long errorCode = Long.parseLong(actionResult.getBodyErr());
            String[] params = Optional.ofNullable(actionResult.getDetailParams())
                    .map(e -> e.toArray(new String[0]))
                    .orElse(new String[0]);
            throw new LegoCheckedException(errorCode, params, "check connection failed.");
        }
    }

    private List<ProtectedResource> queryAllDatabaseInstances(ProtectedResource database,
        List<ProtectedEnvironment> agents) {
        return agents.stream()
                .map(agent -> queryDatabaseInstanceFromAgent(database, agent))
                .collect(Collectors.toList());
    }

    private ProtectedResource queryDatabaseInstanceFromAgent(ProtectedResource database, ProtectedEnvironment agent) {
        ProtectedResource instance = initDatabaseInstance(database, agent);
        oracleBaseService.refreshClusterInstanceActiveStandby(instance, agent);
        return queryInstanceInfoFromAgent(instance, agent);
    }

    private ProtectedResource initDatabaseInstance(ProtectedResource clusterDatabase, ProtectedEnvironment agent) {
        ProtectedResource instance = new ProtectedResource();
        instance.setName(clusterDatabase.getName());
        instance.setAuth(BeanTools.copy(clusterDatabase.getAuth(), Authentication::new));
        instance.setSubType(ResourceSubTypeEnum.ORACLE_CLUSTER_INSTANCE.getType());
        instance.setExtendInfoByKey(DatabaseConstants.HOST_ID, agent.getUuid());
        instance.setExtendInfoByKey(INSTALL_USERNAME, clusterDatabase.getExtendInfoByKey(INSTALL_USERNAME));
        instance.setExtendInfoByKey(ACCESS_ORACLE_HOME, clusterDatabase.getExtendInfoByKey(ACCESS_ORACLE_HOME));
        instance.setExtendInfoByKey(ACCESS_ORACLE_BASE, clusterDatabase.getExtendInfoByKey(ACCESS_ORACLE_BASE));
        return instance;
    }

    private ProtectedResource queryInstanceInfoFromAgent(ProtectedResource instance, ProtectedEnvironment agent) {
        instance.setExtendInfoByKey(OracleConstants.QUERY_TYPE, "rac");
        ListResourceReq request = new ListResourceReq();
        request.setAppEnv(BeanTools.copy(agent, AppEnv::new));
        request.setApplication(BeanTools.copy(instance, Application::new));
        AgentDetailDto response = agentUnifiedService.getDetail(ResourceSubTypeEnum.ORACLE_CLUSTER_INSTANCE.getType(),
                agent.getEndpoint(), agent.getPort(), request);
        List<AppResource> resourceList = response.getResourceList();
        if (VerifyUtil.isEmpty(resourceList) || VerifyUtil.isEmpty(resourceList.get(0).getExtendInfo())) {
            log.error("Get cluster info failed, instance name:{}", instance.getName());
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED, "Query from agent failed");
        }
        Map<String, String> instanceInfo = resourceList.get(0).getExtendInfo();
        if (!instanceInfo.containsKey(OracleConstants.CLUSTER_NAME)) {
            throw new LegoCheckedException(CommonErrorCode.INSTANCES_NOT_BELONG_SAME_CLUSTER, "Database name is empty");
        }
        instance.getExtendInfo().putAll(resourceList.get(0).getExtendInfo());
        return instance;
    }

    private void checkInstanceName(List<ProtectedResource> instances) {
        String databaseName = instances.get(0).getExtendInfo().get(OracleConstants.CLUSTER_NAME);
        for (ProtectedResource instance : instances) {
            if (!databaseName.equals(instance.getExtendInfo().get(OracleConstants.CLUSTER_NAME))) {
                log.error("Instance name are not same, name: {}, {}.", databaseName,
                        instance.getExtendInfo().get(OracleConstants.CLUSTER_NAME));
                throw new LegoCheckedException(CommonErrorCode.INSTANCES_NOT_BELONG_SAME_CLUSTER, "Name not same");
            }
        }
    }

    private void updateClusterDatabase(ProtectedResource database, List<ProtectedResource> instances) {
        String envIdentity = database.getParentUuid() + database.getSubType() + database.getName();
        String uuid = Optional.ofNullable(database.getUuid())
                .orElse(UUID.nameUUIDFromBytes(envIdentity.getBytes(Charset.defaultCharset())).toString());
        database.setUuid(uuid);
        ProtectedResource instance = instances.get(0);
        database.setVersion(instance.getVersion());
        database.setPath(instance.getExtendInfoByKey(OracleConstants.CLUSTER_IP));
        database.setExtendInfoByKey(OracleConstants.INSTANCES,
                JsonUtil.json(instances.stream().map(ProtectedResource::getExtendInfo).collect(Collectors.toList())));
        database.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        database.setExtendInfoByKey(OracleConstants.INST_NAME,
                instance.getExtendInfoByKey(OracleConstants.CLUSTER_NAME));
        database.setExtendInfoByKey(OracleConstants.ORACLE_GROUP,
                instance.getExtendInfoByKey(OracleConstants.ORACLE_GROUP));
        database.setExtendInfoByKey(OracleConstants.VERIFY_STATUS, "true");
    }

    private void updateDatabaseStatusToDb(String uuid, LinkStatusEnum statusEnum) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(uuid);
        protectedResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, statusEnum.getStatus().toString());
        resourceService.updateSourceDirectly(Collections.singletonList(protectedResource));
        log.info("Oracle cluster database health check finish, uuid: {}, status: {}.", uuid, statusEnum);
    }

    private void checkStorage(ProtectedResource resource, List<ProtectedEnvironment> clusterHosts) {
        List<OracleStorage> storages = getStorages(resource);
        if (CollectionUtils.isEmpty(storages)) {
            return;
        }
        List<OracleStorage> authStorages = getAuthStorageInfo(resource);
        if (CollectionUtils.isEmpty(authStorages)) {
            return;
        }

        // 存储数量只能1个
        if (storages.size() > OracleConstants.ORACLE_CLUSTER_STORAGE_MAN_NUM) {
            log.error("Storage count({}) over limit.", storages.size());
            throw new LegoCheckedException(CommonErrorCode.STORAGE_OVER_LIMIT,
                new String[] {String.valueOf(OracleConstants.ORACLE_CLUSTER_STORAGE_MAN_NUM)},
                "Storage count over limit.");
        }

        // 校验ip,端口
        storages.forEach(storage -> {
            StorageCheckUtil.checkIp(storage.getIpList(), resource.getName());
            StorageCheckUtil.checkPort(storage.getPort(), resource.getName());
        });
        // 校验证书
        authStorages.forEach(StorageCheckUtil::checkCert);
        ProtectedResource clone = JSONObject.fromObject(resource).toBean(ProtectedResource.class);
        clone.getExtendInfo().put("isCheckStorage", "true");
        String ipList = authStorages.get(0).getIpList();
        AgentBaseDto response = new AgentBaseDto();
        for (String ip : ipList.split(",")) {
            authStorages.get(0).setIpList(ip);
            clone.getAuth().getExtendInfo().put("storages", JsonUtil.json(authStorages));
            response = agentUnifiedService.checkApplication(clone, clusterHosts.get(0));
            if (OracleConstants.SUCCESS.equals(response.getErrorCode())) {
                break;
            }
        }
        checkConnectResult(response);
    }

    private List<OracleStorage> getStorages(ProtectedResource clusterDatabase) {
        return Optional.of(clusterDatabase)
            .map(ProtectedResource::getExtendInfo)
            .map(extendInfo -> extendInfo.get(STORAGES))
            .map(storage -> JSONArray.toCollection(JSONArray.fromObject(storage), OracleStorage.class))
            .orElse(new ArrayList<>());
    }

    private List<OracleStorage> getAuthStorageInfo(ProtectedResource resource) {
        return Optional.of(resource)
            .map(ProtectedResource::getAuth)
            .map(Authentication::getExtendInfo)
            .map(extendInfo -> extendInfo.get(STORAGES))
            .map(storage -> JSONArray.toCollection(JSONArray.fromObject(storage), OracleStorage.class))
            .orElse(new ArrayList<>());
    }

    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = ResourceFeature.defaultValue();
        // oracle集群数据库名称可以重复，不检查集群数据库重名
        resourceFeature.setShouldCheckResourceNameDuplicate(false);
        return resourceFeature;
    }

    @Override
    public boolean applicable(ProtectedResource resource) {
        return ResourceSubTypeEnum.ORACLE_CLUSTER.equalsSubType(resource.getSubType());
    }
}
