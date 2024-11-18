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
package openbackup.sqlserver.resources.access.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.sqlserver.common.SqlServerConstants;
import openbackup.sqlserver.common.SqlServerErrorCode;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.alibaba.fastjson.JSON;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * sqlserver集群实例provider
 *
 */
@Component
@Slf4j
public class SqlServerClusterInstanceProvider implements ResourceProvider {
    private final AgentUnifiedService agentUnifiedService;

    private final SqlServerBaseService sqlServerBaseService;

    /**
     * 集群实例构造器注入
     *
     * @param agentUnifiedService agent代理属性
     * @param sqlServerBaseService 基础服务
     */
    public SqlServerClusterInstanceProvider(AgentUnifiedService agentUnifiedService,
        SqlServerBaseService sqlServerBaseService) {
        this.agentUnifiedService = agentUnifiedService;
        this.sqlServerBaseService = sqlServerBaseService;
    }

    /**
     * 资源重名检查配置
     *
     * @return SQL Server实例注册重名检查配置
     */
    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = ResourceProvider.super.getResourceFeature();

        // SQL Server实例名称可以重复，不检查实例重名
        resourceFeature.setShouldCheckResourceNameDuplicate(false);
        // SQL Server集群实例扫描不需要刷新主机信息
        resourceFeature.setShouldUpdateDependencyHostInfoWhenScan(false);
        return resourceFeature;
    }

    /**
     * 资源删除前的处理
     *
     * @param resource ProtectedResource
     * @return ResourceDeleteContext
     */
    @Override
    public ResourceDeleteContext preHandleDelete(ProtectedResource resource) {
        log.info("[SQL Server Delete] cluster instance uuid: {} delete start, cluster uuid: {}", resource.getUuid(),
            resource.getRootUuid());
        List<ResourceDeleteContext.ResourceDeleteDependency> resourceDeleteDependencies = new ArrayList<>();
        ResourceDeleteContext.ResourceDeleteDependency clusterInstance =
            new ResourceDeleteContext.ResourceDeleteDependency();
        clusterInstance.setShouldCheckIfBeDependency(false);
        clusterInstance.setDeleteIds(Collections.singletonList(resource.getUuid()));
        resourceDeleteDependencies.add(clusterInstance);
        List<ProtectedResource> alwaysOns = sqlServerBaseService.getResourceOfClusterByType(
            resource.getRootUuid(), ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType(), false);
        if (!VerifyUtil.isEmpty(alwaysOns)) {
            List<String> dependentAlwaysOnIdList = alwaysOns.stream()
                .filter(agGroup -> Optional.ofNullable(agGroup.getDependencies())
                    .orElse(new HashMap<>())
                    .getOrDefault(SqlServerConstants.INSTANCE, new ArrayList<>())
                    .stream()
                    .map(ProtectedResource::getUuid)
                    .collect(Collectors.toList())
                    .contains(resource.getUuid()))
                .map(ProtectedResource::getUuid)
                .collect(Collectors.toList());
            log.info("[SQL Server Delete] cluster instance uuid: {} delete, dependent alwaysOn size: {}",
                resource.getUuid(), dependentAlwaysOnIdList);
            ResourceDeleteContext.ResourceDeleteDependency alwaysOnToDelete =
                new ResourceDeleteContext.ResourceDeleteDependency();
            alwaysOnToDelete.setShouldCheckIfBeDependency(false);
            alwaysOnToDelete.setDeleteIds(dependentAlwaysOnIdList);
            resourceDeleteDependencies.add(alwaysOnToDelete);
        }
        ResourceDeleteContext resourceDeleteContext = new ResourceDeleteContext();
        resourceDeleteContext.setResourceDeleteDependencyList(resourceDeleteDependencies);
        return resourceDeleteContext;
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.info("start create sqlserver cluster instance check. resource name: {}", resource.getName());
        // 校验是否已经存在实例
        sqlServerBaseService.checkInstanceExist(resource);
        List<ProtectedResource> nodes = getAgentsOfProtectedResource(resource);
        checkClusterInstanceNodeNum(nodes);
        Map<String, String> extendInfo = Optional.ofNullable(resource.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(DatabaseConstants.AGENTS, JSON.toJSONString(nodes));
        resource.setExtendInfo(extendInfo);

        // 检查节点是否已注册
        checkHostRegistered(resource.getParentUuid(), nodes);

        List<ProtectedEnvironment> hosts = sqlServerBaseService.getProtectedEnvironmentByResourceList(nodes);

        // 检查节点是否均在线
        checkNodeOnline(hosts);

        // 校验各节点是否属于本注册集群实例
        checkNodesBelongToClusterInstance(resource, hosts);

        // 设置集群实例各项参数
        setClusterInstanceValues(resource, hosts);
    }

    private List<ProtectedResource> getAgentsOfProtectedResource(ProtectedResource resource) {
        return Optional.ofNullable(resource.getDependencies())
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.ERR_PARAM, "[SQL Server] cluster nodes not found"))
            .get(DatabaseConstants.AGENTS);
    }

    private void setClusterInstanceValues(ProtectedResource clusterInstance, List<ProtectedEnvironment> hosts) {
        // 与数据库插件对齐下属主机名列表
        clusterInstance.getExtendInfo().put(DatabaseConstants.END_POINT, hosts.stream()
            .map(ProtectedEnvironment::getName)
            .collect(Collectors.joining(DatabaseConstants.SPLIT_CHAR)));

        // 复制需要使用集群实例的位置
        clusterInstance.setPath(hosts.stream()
            .map(ProtectedEnvironment::getEndpoint)
            .collect(Collectors.joining(DatabaseConstants.SPLIT_CHAR)));
    }

    private void checkHostRegistered(String clusterUuid, List<ProtectedResource> hosts) {
        List<String> clusterHostUuidId = sqlServerBaseService.getResourceOfClusterByType(clusterUuid,
            ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType(), false)
            .stream()
            .map(instance -> instance.getDependencies()
                .get(DatabaseConstants.AGENTS)
                .stream()
                .map(ProtectedResource::getUuid)
                .collect(Collectors.toList()))
            .flatMap(Collection::stream)
            .collect(Collectors.toList());
        hosts.forEach(host -> {
            if (clusterHostUuidId.contains(host.getUuid())) {
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED,
                    "[SQL Server] cluster nodes has registered");
            }
        });
    }

    private void checkClusterInstanceNodeNum(List<ProtectedResource> nodes) {
        int nodeSize = nodes.size();
        if (nodeSize < SqlServerConstants.CLUSTER_INSTANCE_MIN_NODE_NUM
            || nodeSize > SqlServerConstants.CLUSTER_INSTANCE_MAX_NODE_NUM) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "[SQL Server] cluster nodes num error");
        }
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("[SQL Server Cluster Instance Update] cluster uuid: {} instance({}) uuid: {} before update start",
            resource.getParentUuid(), resource.getName(), resource.getUuid());
        ProtectedResource clusterInstance = sqlServerBaseService.getResourceByUuid(resource.getUuid());
        if (!clusterInstance.getParentUuid().equals(resource.getParentUuid())) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
                "[SQL Server Cluster Update] cluster instance parent can not change.");
        }
        List<String> nodeIds = getAgentsOfProtectedResource(clusterInstance).stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        List<ProtectedResource> nodes = getAgentsOfProtectedResource(clusterInstance);
        List<String> clusterInstanceNodeIds = nodes.stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        if (clusterInstanceNodeIds.size() != nodeIds.size() || !clusterInstanceNodeIds.containsAll(nodeIds)) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
                "[SQL Server Cluster Instance Update] cluster instance nodes can not change.");
        }
        List<ProtectedEnvironment> hosts = sqlServerBaseService.getProtectedEnvironmentByResourceList(nodes);
        checkNodeOnline(hosts);
        log.info("[SQL Server Cluster Instance Update] cluster uuid: {} instance({}) uuid: {} agent check start",
            resource.getParentUuid(), resource.getName(), resource.getUuid());
        checkNodesBelongToClusterInstance(resource, hosts);
        setClusterInstanceValues(resource, hosts);
        log.info("[SQL Server Cluster Instance Update] cluster uuid: {} instance({}) uuid: {} before update end",
            resource.getParentUuid(), resource.getName(), resource.getUuid());
    }

    private void checkNodeOnline(List<ProtectedEnvironment> hosts) {
        if (hosts.stream()
            .anyMatch(host -> LinkStatusEnum.OFFLINE.getStatus().toString()
            .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(host)))) {
            throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, "[SQL Server] cluster agent is offLine!");
        }
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return Objects.nonNull(protectedResource) && Objects.equals(protectedResource.getSubType(),
            ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType());
    }

    private void checkNodesBelongToClusterInstance(ProtectedResource clusterInstance,
        List<ProtectedEnvironment> hosts) {
        // 对所有节点进行检查并获取检查结果
        List<AgentBaseDto> checkResultList = hosts.stream()
            .map(protectedResource -> getCheckResultByAgent(clusterInstance, protectedResource))
            .collect(Collectors.toList());

        // 检查结果分析
        if (!analysisClusterInstanceByCheckResult(checkResultList)) {
            log.error("[SQL Server] check cluster instance({}) failed", clusterInstance.getName());
            throw new LegoCheckedException(SqlServerErrorCode.CHECK_CLUSTER_NUM_FAILED, "cluster nodes check error");
        }
        addMasterNodeForClusterNode(clusterInstance, checkResultList, hosts);
    }

    private void addMasterNodeForClusterNode(ProtectedResource resource, List<AgentBaseDto> checkResultList,
        List<ProtectedEnvironment> nodes) {
        int resultSize = checkResultList.size();
        Map<String, String> extendInfo = resource.getExtendInfo();

        // 仅一个节点直接设计为主节点
        if (resultSize == SqlServerConstants.CLUSTER_INSTANCE_MIN_NODE_NUM) {
            extendInfo.put(SqlServerConstants.MASTER_NODE, nodes.get(0).getUuid());
            return;
        }

        // 有两个节点选择为节点角色为1的作为主节点
        if (resultSize == SqlServerConstants.CLUSTER_INSTANCE_MAX_NODE_NUM) {
            Object roleOfFirstNode = getValueOfCheckResultByKey(checkResultList.get(0), DatabaseConstants.ROLE);
            if (roleOfFirstNode instanceof Integer && (Integer) roleOfFirstNode == 1) {
                extendInfo.put(SqlServerConstants.MASTER_NODE, nodes.get(0).getUuid());
                return;
            }
            Object roleOfSecondNode = getValueOfCheckResultByKey(checkResultList.get(1), DatabaseConstants.ROLE);
            if (roleOfSecondNode instanceof Integer && (Integer) roleOfSecondNode == 1) {
                extendInfo.put(SqlServerConstants.MASTER_NODE, nodes.get(1).getUuid());
            }
        }
        log.error("[SQL Server] analysisClusterInstanceByCheckResult result size error: {}", resultSize);
    }

    private boolean analysisClusterInstanceByCheckResult(List<AgentBaseDto> checkResultList) {
        int resultSize = checkResultList.size();
        boolean isFirstVirtualServiceRun = (boolean) getValueOfCheckResultByKey
            (checkResultList.get(0), DatabaseConstants.STATE);
        if (resultSize == SqlServerConstants.CLUSTER_INSTANCE_MIN_NODE_NUM
            && checkResultList.get(0).getErrorMessage() != null) {
            if (isFirstVirtualServiceRun) {
                return true;
            }
            Object virtualServiceName = getValueOfCheckResultByKey(checkResultList.get(0), DatabaseConstants.NAME);
            log.warn("[SQL Server] one node not running service to cluster: {}, not allow to create cluster instance",
                virtualServiceName);
            return false;
        }
        if (resultSize == SqlServerConstants.CLUSTER_INSTANCE_MAX_NODE_NUM) {
            boolean isSecondVirtualServiceRun = (boolean) getValueOfCheckResultByKey
                (checkResultList.get(1), DatabaseConstants.STATE);
            Object virtualServiceNameOfFirstNode = getValueOfCheckResultByKey(checkResultList.get(0),
                DatabaseConstants.NAME);
            Object virtualServiceNameOfSecondNode = getValueOfCheckResultByKey(checkResultList.get(1),
                DatabaseConstants.NAME);
            if (!isFirstVirtualServiceRun && !isSecondVirtualServiceRun) {
                log.error("[SQL Server] both two node not running service. Cluster is {}.",
                    virtualServiceNameOfFirstNode);
                return false;
            }
            return !VerifyUtil.isEmpty(virtualServiceNameOfFirstNode) && virtualServiceNameOfFirstNode.equals(
                virtualServiceNameOfSecondNode);
        }
        log.error("[SQL Server] analysisClusterInstanceByCheckResult result size error: {}", resultSize);
        return false;
    }

    private Object getValueOfCheckResultByKey(AgentBaseDto result, String key) {
        if (!result.isAgentBaseDtoReturnSuccess()) {
            log.error("[SQL Server] check node belong to cluster failed");
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "get virtualServiceName from agent fail");
        }
        String message = Optional.ofNullable(result.getErrorMessage())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
                "get virtualServiceName from agent fail"));
        return Optional.ofNullable(JSONObject.fromObject(message).get(key))
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.ERR_PARAM, "node not belong to cluster instance"));
    }

    // 集群校验并返回校验结果
    private AgentBaseDto getCheckResultByAgent(ProtectedResource clusterInstance, ProtectedEnvironment environment) {
        AgentBaseDto checkResult = agentUnifiedService.checkApplication(clusterInstance, environment);
        String errorCode = checkResult.getErrorCode();
        log.info("[SQL Server] cluster instance: {} host name: {} checkResult: {}, message: {}",
            clusterInstance.getName(), environment.getName(), errorCode, checkResult.getErrorMessage());
        if (!String.valueOf(SqlServerErrorCode.AGENT_RETURN_CODE_SUCCESS).equals(errorCode)) {
            JSONObject jsonObject = JSONObject.fromObject(checkResult.getErrorMessage());
            String bodyErr = jsonObject.getString("bodyErr");
            if (!VerifyUtil.isEmpty(bodyErr)) {
                throw new LegoCheckedException(Long.parseLong(bodyErr), "Cluster instance check fail.");
            }
            log.error("[SQL Server] cluster instance: {} host name: {} checkResult: {}, agent return no bodyError.",
                clusterInstance.getName(), environment.getName(), errorCode);
            throw new LegoCheckedException(Long.parseLong(errorCode), "Cluster instance check by agent return code.");
        }
        return checkResult;
    }
}