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
package openbackup.db2.protection.access.service.impl;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.enums.NodeType;
import openbackup.db2.protection.access.constant.Db2Constants;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.enums.Db2HadrRoleEnum;
import openbackup.db2.protection.access.enums.Db2ResourceStatusEnum;
import openbackup.db2.protection.access.service.Db2InstanceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.RequestUriUtil;
import openbackup.system.base.util.StreamUtil;

import com.google.common.collect.ImmutableList;

import feign.FeignException;
import io.jsonwebtoken.lang.Collections;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.net.URI;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * db2实例服务
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-03
 */
@Service
@Slf4j
public class Db2InstanceServiceImpl implements Db2InstanceService {
    private final ResourceService resourceService;

    private final ProtectedEnvironmentService environmentService;

    private final AgentUnifiedService agentUnifiedService;

    public Db2InstanceServiceImpl(ResourceService resourceService, ProtectedEnvironmentService environmentService,
        AgentUnifiedService agentUnifiedService) {
        this.resourceService = resourceService;
        this.environmentService = environmentService;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public void checkSingleInstanceIsRegistered(ProtectedResource resource) {
        int instanceCount = querySingleInstanceNums(resource);
        if (instanceCount != IsmNumberConstant.ZERO) {
            log.error("This db2 single instance is registered. name: {}", resource.getName());
            throw new LegoCheckedException(DatabaseErrorCode.INSTANCE_HAS_REGISTERED,
                "This db2 single instance is registered.");
        }
    }

    @Override
    public void checkSingleInstanceNameIsChanged(ProtectedResource resource) {
        int instanceCount = querySingleInstanceNums(resource);
        if (instanceCount != IsmNumberConstant.ONE) {
            log.error("This db2 single instance name is changed. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This db2 single instance name is changed.");
        }
    }

    @Override
    public AgentBaseDto checkIsClusterInstance(ProtectedResource resource) {
        AppEnv appEnv = buildAppEnv(resource);
        Application application = buildApplication(resource);
        return checkClusterInstanceByAgent(appEnv, application);
    }

    @Override
    public void checkClusterInstanceIsRegistered(ProtectedResource resource) {
        int instanceCount = queryClusterInstanceNums(resource);
        if (instanceCount != IsmNumberConstant.ZERO) {
            log.error("This db2 cluster instance is registered. name: {}", resource.getName());
            throw new LegoCheckedException(DatabaseErrorCode.INSTANCE_HAS_REGISTERED,
                "This cluster instance is registered.");
        }
    }

    private int queryClusterInstanceNums(ProtectedResource resource) {
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.NAME, resource.getName());
        filter.put(DatabaseConstants.SUB_TYPE, resource.getSubType());
        filter.put(DatabaseConstants.PARENT_UUID, resource.getParentUuid());
        return resourceService.query(IsmNumberConstant.ZERO, IsmNumberConstant.ONE, filter).getTotalCount();
    }

    @Override
    public void checkClusterInstanceNameIsChanged(ProtectedResource resource) {
        int instanceCount = queryClusterInstanceNums(resource);
        if (instanceCount != IsmNumberConstant.ONE) {
            log.error("This db2 cluster instance name is changed. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This db2 cluster instance name is changed.");
        }
    }

    @Override
    public void filterClusterInstance(ProtectedResource clusterInstance) {
        if (!Db2ClusterTypeEnum.POWER_HA.getType()
            .equals(clusterInstance.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE))) {
            return;
        }
        ProtectedResource primaryInstance = queryHadrPrimaryNode(clusterInstance);
        clusterInstance.getDependencies().put(DatabaseConstants.CHILDREN, ImmutableList.of(primaryInstance));
    }

    private ProtectedResource queryHadrPrimaryNode(ProtectedResource clusterInstance) {
        List<ProtectedResource> childrenInstance = clusterInstance.getDependencies().get(DatabaseConstants.CHILDREN);
        for (ProtectedResource subInstance : childrenInstance) {
            ProtectedEnvironment subEnv = environmentService.getEnvironmentById(
                subInstance.getExtendInfoByKey(DatabaseConstants.HOST_ID));
            AppEnvResponse appEnvResponse = agentUnifiedService.getClusterInfo(subInstance, subEnv);
            List<NodeInfo> nodes = appEnvResponse.getNodes();
            if (!Collections.isEmpty(nodes)) {
                NodeInfo masterNode = nodes.stream()
                    .filter(
                        node -> NodeType.MASTER.getNodeType().equals(node.getExtendInfo().get(DatabaseConstants.ROLE)))
                    .findFirst()
                    .orElse(null);
                if (masterNode != null) {
                    return subInstance;
                }
            }
        }
        log.error("The primary node is not queried. uuid: {}.", clusterInstance.getUuid());
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "The primary node is not queried");
    }

    @Override
    public void checkHadrClusterInstance(ProtectedResource resource) {
        Set<String> versions = getHadrVersion(resource);
        if (versions.size() != IsmNumberConstant.ONE) {
            log.error("The database version is inconsistent. resource id: {}, name: {} versions: {}",
                resource.getUuid(), resource.getName(), versions);
            throw new LegoCheckedException(DatabaseErrorCode.DATABASE_VERSION_INCONSISTENT,
                "The database version is inconsistent.");
        }
        Set<String> deployOs = getHadrDeployOs(resource);
        if (deployOs.size() != IsmNumberConstant.ONE) {
            log.error("The database deploy os is inconsistent. resource id: {}, name: {}, deployOs: {}",
                resource.getUuid(), resource.getName(), versions);
            throw new LegoCheckedException(DatabaseErrorCode.DATABASE_OS_INCONSISTENT,
                "The database deploy os is inconsistent.");
        }
        Set<String> databaseBits = getHadrDatabaseBits(resource);
        if (databaseBits.size() != IsmNumberConstant.ONE) {
            log.error("The database bit size is inconsistent.. resource id: {}, name: {}, deployOs: {}",
                resource.getUuid(), resource.getName(), databaseBits);
            throw new LegoCheckedException(DatabaseErrorCode.DATABASE_BITS_INCONSISTENT,
                "The database bit size is inconsistent.");
        }
        resource.setVersion(versions.iterator().next());
        resource.setExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY, deployOs.iterator().next());
        resource.setExtendInfoByKey(DatabaseConstants.DATABASE_BITS_KEY, databaseBits.iterator().next());
    }

    @Override
    public List<ProtectedResource> scanDatabase(ProtectedResource clusterInstance, ProtectedEnvironment environment) {
        if (Db2ClusterTypeEnum.HADR.getType()
            .equals(clusterInstance.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE))) {
            return scanHadrDatabase(clusterInstance, environment);
        }
        return scanDpfAndHaDatabase(clusterInstance, environment);
    }

    private List<ProtectedResource> scanHadrDatabase(ProtectedResource clusterInstance,
        ProtectedEnvironment environment) {
        List<AppResource> databases = queryHadrDatabase(clusterInstance);
        return buildHadrDatabase(databases, clusterInstance, environment);
    }

    private List<AppResource> queryHadrDatabase(ProtectedResource clusterInstance) {
        List<AppResource> databases = new ArrayList<>();
        clusterInstance.getDependencies().get(DatabaseConstants.CHILDREN).forEach(subInstance -> {
            ProtectedEnvironment subEnv = extractEnvironmentByInstance(subInstance);
            checkInstanceStatus(subInstance, subEnv);
            AgentDetailDto databaseDetail = queryDatabaseByAgent(subEnv, subInstance);
            Optional.ofNullable(databaseDetail.getResourceList())
                .ifPresent(databaseList -> databases.addAll(databaseList));
        });
        return databases;
    }

    private void checkInstanceStatus(ProtectedResource subInstance, ProtectedEnvironment subEnv) {
        URI uri = RequestUriUtil.getRequestUri(subEnv.getEndpoint(), subEnv.getPort());
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(BeanTools.copy(subEnv, AppEnv::new));
        checkAppReq.setApplication(BeanTools.copy(subInstance, Application::new));
        AgentBaseDto checkResult;
        try {
            checkResult = agentUnifiedService.check(ResourceSubTypeEnum.DB2_INSTANCE.getType(), subEnv, checkAppReq);
        } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
            log.error("This instance check connection fail.");
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "check failed.");
        } finally {
            if (Objects.nonNull(subInstance.getAuth())) {
                StringUtil.clean(subInstance.getAuth().getAuthPwd());
            }
        }
        if (Long.parseLong(checkResult.getErrorCode()) == DatabaseConstants.SUCCESS_CODE) {
            return;
        }
        throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "check failed.");
    }

    private ProtectedEnvironment extractEnvironmentByInstance(ProtectedResource subInstance) {
        return subInstance.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException("This instance environment is empty."));
    }

    private AgentDetailDto queryDatabaseByAgent(ProtectedEnvironment subEnv, ProtectedResource subInstance) {
        ListResourceReq db2DbsRequest = new ListResourceReq();
        db2DbsRequest.setAppEnv(BeanTools.copy(subEnv, AppEnv::new));
        db2DbsRequest.setApplication(BeanTools.copy(subInstance, Application::new));
        return agentUnifiedService.getDetail(ResourceSubTypeEnum.DB2_INSTANCE.getType(), subEnv.getEndpoint(),
            subEnv.getPort(), db2DbsRequest);
    }

    private List<ProtectedResource> buildHadrDatabase(List<AppResource> databases, ProtectedResource clusterInstance,
        ProtectedEnvironment environment) {
        Map<String, List<AppResource>> databaseMap = databases.stream()
            .collect(Collectors.groupingBy(AppResource::getName));
        List<ProtectedResource> resources = new ArrayList<>();
        String agents = extractAgents(environment);
        int nodeNums = environment.getDependencies().get(DatabaseConstants.AGENTS).size();
        for (Map.Entry<String, List<AppResource>> entry : databaseMap.entrySet()) {
            if (!checkHadrDatabase(entry.getValue(), nodeNums)) {
                continue;
            }
            Optional<AppResource> primaryDatabase = getPrimaryDatabase(entry.getValue());
            primaryDatabase.ifPresent(database -> {
                resources.add(convertHadrDatabase(database, entry.getValue(), clusterInstance, environment, agents));
            });
        }
        addLockedHadrDatabases(resources, clusterInstance.getUuid());
        return resources;
    }

    private void addLockedHadrDatabases(List<ProtectedResource> resources, String clusterInstanceId) {
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.PARENT_UUID, clusterInstanceId);
        filter.put(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.HADR.getType());
        filter.put(DatabaseConstants.STATUS,
            Arrays.asList(Db2ResourceStatusEnum.RESTORING.getStatus(), Db2ResourceStatusEnum.BACKUPING.getStatus()));
        int pageNo = 0;
        List<ProtectedResource> lockedDatabases = new ArrayList<>();
        PageListResponse<ProtectedResource> databases;
        do {
            databases = resourceService.query(pageNo, IsmNumberConstant.HUNDRED, filter);
            lockedDatabases.addAll(databases.getRecords());
            pageNo++;
        } while (databases.getRecords().size() >= IsmNumberConstant.HUNDRED);
        List<String> resourceIds = resources.stream().map(ProtectedResource::getUuid).collect(Collectors.toList());
        for (ProtectedResource lockedDatabase : lockedDatabases) {
            if (!resourceIds.contains(lockedDatabase.getUuid())) {
                log.info("Db2 hadr scan database locked id: {}, name: {}.", lockedDatabase.getUuid(),
                    lockedDatabase.getName());
                resources.add(lockedDatabase);
            }
        }
    }

    private boolean checkHadrDatabase(List<AppResource> databases, int nodeNums) {
        if (databases.size() != nodeNums) {
            return false;
        }
        List<AppResource> primaryDatabases = databases.stream()
            .filter(database -> Db2HadrRoleEnum.PRIMARY.getRole()
                .equals(database.getExtendInfo().get(Db2Constants.HADR_ROLE_KEY)))
            .collect(Collectors.toList());
        if (primaryDatabases.size() != IsmNumberConstant.ONE) {
            return false;
        }
        return isMatchHadr(databases);
    }

    private boolean isMatchHadr(List<AppResource> databases) {
        Map<String, String> hostMap = new HashMap<>();
        databases.forEach(database -> {
            String localHost = database.getExtendInfo().get(Db2Constants.HADR_LOCAL_HOST_KEY);
            String remoteHost = database.getExtendInfo().get(Db2Constants.HADR_REMOTE_HOST_KEY);
            hostMap.put(localHost, remoteHost);
        });
        for (Map.Entry<String, String> entry : hostMap.entrySet()) {
            if (!hostMap.containsKey(entry.getValue())) {
                return false;
            }
        }
        return true;
    }

    private Optional<AppResource> getPrimaryDatabase(List<AppResource> databases) {
        return databases.stream()
            .filter(database -> Db2HadrRoleEnum.PRIMARY.getRole()
                .equals(database.getExtendInfo().get(Db2Constants.HADR_ROLE_KEY)))
            .findFirst();
    }

    private ProtectedResource convertHadrDatabase(AppResource database, List<AppResource> databaseList,
        ProtectedResource clusterInstance, ProtectedEnvironment environment, String agents) {
        ProtectedResource databaseResource = BeanTools.copy(database, ProtectedResource::new);
        databaseResource.setExtendInfo(new HashMap<>());
        databaseResource.setUuid(getUniqueUUID(databaseList));
        databaseResource.setParentUuid(clusterInstance.getUuid());
        databaseResource.setParentName(clusterInstance.getName());
        databaseResource.setRootUuid(environment.getUuid());
        databaseResource.setPath(environment.getEndpoint());
        databaseResource.setVersion(clusterInstance.getVersion());
        databaseResource.setExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY,
            clusterInstance.getExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY));
        databaseResource.setExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE,
            clusterInstance.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE));
        databaseResource.setExtendInfoByKey(Db2Constants.NODE_DATABASE_KEY,
            JSONArray.fromObject(databaseList.stream().collect(Collectors.toList())).toString());
        databaseResource.setExtendInfoByKey(DatabaseConstants.AGENTS, agents);
        databaseResource.setExtendInfoByKey(Db2Constants.CATALOG_IP_KEY,
            database.getExtendInfo().get(Db2Constants.HADR_LOCAL_HOST_KEY));
        return databaseResource;
    }

    private String getUniqueUUID(List<AppResource> databaseList) {
        String envIdentity = databaseList.stream()
            .map(AppResource::getUuid)
            .sorted()
            .collect(Collectors.joining(DatabaseConstants.SPLIT_CHAR));
        return UUID.nameUUIDFromBytes(envIdentity.getBytes(Charset.defaultCharset())).toString();
    }

    private List<ProtectedResource> scanDpfAndHaDatabase(ProtectedResource clusterInstance,
        ProtectedEnvironment environment) {
        filterClusterInstance(clusterInstance);
        ProtectedResource subInstance = extractSubInstance(clusterInstance);
        ProtectedEnvironment subEnv = extractEnvironmentByInstance(subInstance);
        checkInstanceStatus(subInstance, subEnv);
        AgentDetailDto databaseDetail = queryDatabaseByAgent(subEnv, subInstance);
        return convertDatabase(databaseDetail, clusterInstance, environment);
    }

    private ProtectedResource extractSubInstance(ProtectedResource clusterInstance) {
        ProtectedResource subInstance = clusterInstance.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException("Don't have sub instance."));
        // powerHA集群实例时需要把集群实例的id设置到子实例上，为了powerHA主备切换时扫描的数据库的id不变
        if (Db2ClusterTypeEnum.POWER_HA.getType()
            .equals(clusterInstance.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE))) {
            subInstance.setUuid(clusterInstance.getUuid());
        }
        return subInstance;
    }

    private List<ProtectedResource> convertDatabase(AgentDetailDto databaseDetail, ProtectedResource clusterInstance,
        ProtectedEnvironment environment) {
        String agents = extractAgents(environment);
        List<ProtectedResource> resources = new ArrayList<>();
        Optional.ofNullable(databaseDetail.getResourceList())
            .ifPresent(databaseList -> databaseList.forEach(resource -> {
                ProtectedResource databaseResource = BeanTools.copy(resource, ProtectedResource::new);
                databaseResource.setParentName(clusterInstance.getName());
                databaseResource.setParentUuid(clusterInstance.getUuid());
                databaseResource.setRootUuid(environment.getUuid());
                databaseResource.setPath(environment.getEndpoint());
                databaseResource.setVersion(clusterInstance.getVersion());
                databaseResource.setExtendInfoByKey(DatabaseConstants.AGENTS, agents);
                databaseResource.setExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY,
                    clusterInstance.getExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY));
                databaseResource.setExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE,
                    clusterInstance.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE));
                resources.add(databaseResource);
            }));
        return resources;
    }

    private String extractAgents(ProtectedEnvironment environment) {
        return environment.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(ResourceBase::getUuid)
            .collect(Collectors.joining(";"));
    }

    private Set<String> getHadrVersion(ProtectedResource resource) {
        return resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .map(ProtectedResource::getVersion)
            .collect(Collectors.toSet());
    }

    private Set<String> getHadrDeployOs(ProtectedResource resource) {
        return resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .map(this::extractDeployOs)
            .collect(Collectors.toSet());
    }

    private Set<String> getHadrDatabaseBits(ProtectedResource resource) {
        return resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .map(this::extractDatabaseBits)
            .collect(Collectors.toSet());
    }

    private String extractDeployOs(ProtectedResource subInstance) {
        return subInstance.getExtendInfoByKey(DatabaseConstants.DEPLOY_OS_KEY);
    }

    private String extractDatabaseBits(ProtectedResource subInstance) {
        return subInstance.getExtendInfoByKey(DatabaseConstants.DATABASE_BITS_KEY);
    }

    private AppEnv buildAppEnv(ProtectedResource resource) {
        ProtectedResource subInstance = resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This sub instance is empty."));
        ProtectedEnvironment fullAgent = environmentService.getEnvironmentById(
            subInstance.getExtendInfoByKey(DatabaseConstants.HOST_ID));
        AppEnv appEnv = BeanTools.copy(fullAgent, AppEnv::new);
        appEnv.setExtendInfo(buildAppEnvExtendInfo(appEnv, resource));
        appEnv.setSubType(resource.getEnvironment().getSubType());
        return appEnv;
    }

    private Map<String, String> buildAppEnvExtendInfo(AppEnv appEnv, ProtectedResource resource) {
        Map<String, String> extendInfo = Optional.ofNullable(appEnv.getExtendInfo()).orElseGet(HashMap::new);
        extendInfo.put(DatabaseConstants.ALL_NODES, getAllNodes(resource));
        return extendInfo;
    }

    private String getAllNodes(ProtectedResource resource) {
        return resource.getEnvironment()
            .getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(this::getIpList)
            .collect(Collectors.joining(DatabaseConstants.SPLIT_CHAR));
    }

    private String getIpList(ProtectedResource agent) {
        return agent.getExtendInfoByKey(ResourceConstants.AGENT_IP_LIST);
    }

    private Application buildApplication(ProtectedResource resource) {
        ProtectedResource subInstance = resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This sub instance is empty."));
        Application application = BeanTools.copy(subInstance, Application::new);
        application.setSubType(resource.getSubType());
        subInstance.setExtendInfoByKey(DatabaseConstants.NODE_NUMS_KEY,
            String.valueOf(resource.getDependencies().get(DatabaseConstants.CHILDREN).size()));
        return application;
    }

    private AgentBaseDto checkClusterInstanceByAgent(AppEnv appEnv, Application application) {
        CheckAppReq appReq = new CheckAppReq();
        appReq.setAppEnv(appEnv);
        appReq.setApplication(application);
        try {
            return agentUnifiedService.check(application.getSubType(), appEnv.getEndpoint(), appEnv.getPort(), appReq);
        } catch (LegoCheckedException e) {
            long errorCode = CommonErrorCode.AGENT_NETWORK_ERROR;
            if (!VerifyUtil.isEmpty(e.getMessage())) {
                ActionResult actionResult = JSONObject.toBean(e.getMessage(), ActionResult.class);
                errorCode = Long.parseLong(actionResult.getBodyErr());
            }
            log.error("Db2 cluster instance check fail. name: {}", appEnv.getName());
            throw new LegoCheckedException(errorCode, "Db2 cluster instance check fail.");
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Db2 cluster instance check fail. name: {}", appEnv.getName());
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Db2 cluster instance check fail.");
        }
    }

    private int querySingleInstanceNums(ProtectedResource resource) {
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.NAME, resource.getName());
        filter.put(DatabaseConstants.SUB_TYPE, resource.getSubType());
        filter.put(DatabaseConstants.HOST_ID, resource.getExtendInfo().get(DatabaseConstants.HOST_ID));
        return resourceService.query(IsmNumberConstant.ZERO, IsmNumberConstant.ONE, filter).getTotalCount();
    }
}
