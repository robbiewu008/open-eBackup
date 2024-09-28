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
package openbackup.mysql.resources.access.provider;

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
import openbackup.database.base.plugin.enums.ClusterInstanceOnlinePolicy;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.common.MysqlErrorCode;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.host.HostRestApi;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicReference;
import java.util.stream.Collectors;

/**
 * 集群环境健康检查
 *
 */
@Component
@Slf4j
public class MysqlClusterEnvironmentProvider extends DatabaseEnvironmentProvider {
    private final MysqlBaseService mysqlBaseService;

    private final ResourceService resourceService;

    private final MysqlDatabaseScanner mysqlDatabaseScanner;

    private InstanceResourceService instanceResourceService;

    private MysqlInstanceProvider mysqlInstanceProvider;

    private HostRestApi hostRestApi;

    /**
     * MySQLClusterEnvironmentProvider 构造器
     *
     * @param providerManager provider管理器，获取bean和过滤bean
     * @param pluginConfigManager 插件配置管理器
     * @param resourceService 资源服务
     * @param mysqlDatabaseScanner 扫描数据库
     * @param mysqlBaseService mysql通用service
     */
    public MysqlClusterEnvironmentProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        ResourceService resourceService, MysqlDatabaseScanner mysqlDatabaseScanner, MysqlBaseService mysqlBaseService) {
        super(providerManager, pluginConfigManager);
        this.resourceService = resourceService;
        this.mysqlDatabaseScanner = mysqlDatabaseScanner;
        this.mysqlBaseService = mysqlBaseService;
    }

    @Autowired
    public void setInstanceResourceService(InstanceResourceService instanceResourceService) {
        this.instanceResourceService = instanceResourceService;
    }

    @Autowired
    public void setMysqlInstanceProvider(MysqlInstanceProvider mysqlInstanceProvider) {
        this.mysqlInstanceProvider = mysqlInstanceProvider;
    }

    @Autowired
    public void setHostRestApi(HostRestApi hostRestApi) {
        this.hostRestApi = hostRestApi;
    }

    /**
     * mysql集群环境注册provider过滤接口
     *
     * @param resourceSubType 受保护资源subType
     * @return boolean
     */
    @Override
    public boolean applicable(String resourceSubType) {
        return MysqlResourceSubTypeEnum.MYSQL_CLUSTER.getType().equals(resourceSubType);
    }

    /**
     * 扫描受保护环境， 可选实现。根据受保护保护环境决定是否实现该接口，
     * 如受保护的资源需要在ProtectManager进行持久化，则需实现，如VMware虚拟化环境；
     * 如不需要将资源在ProtectManager中实现，则无须实现，比如HDFS的目录、文件，HBase的命名空间，表等。
     *
     * @param environment 受保护环境(集群环境)
     * @return 受保护环境中的资源列表
     */
    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        List<ProtectedResource> resources = new ArrayList<>();
        List<ProtectedResource> agents = environment.getDependencies().get(DatabaseConstants.AGENTS);
        AtomicReference<String> version = new AtomicReference<>("");
        agents.forEach(agent -> {
            ProtectedEnvironment env = mysqlBaseService.getEnvironmentById(agent.getUuid());
            List<ProtectedResource> resourceLists = mysqlDatabaseScanner.scan(env);
            if (!resourceLists.isEmpty()) {
                version.set(resourceLists.get(0).getVersion());
            }
            log.info("mysql single instance finish scan. env id: {}", env.getUuid());
            resources.addAll(resourceLists);
        });
        Map<String, Object> cons = new HashMap<>();
        cons.put(DatabaseConstants.PARENT_UUID, environment.getUuid());
        PageListResponse<ProtectedResource> clusterInstanceQuery = resourceService.query(0, 1, cons);
        ProtectedResource clusterInstance = clusterInstanceQuery.getRecords().get(0);
        clusterInstance.setVersion(version.get());
        boolean isAllChildResUnAuth = resources.stream()
            .filter(resource -> ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType().equals(resource.getSubType())
                && resource.getParentUuid().equals(clusterInstance.getUuid()))
            .allMatch(res -> LinkStatusEnum.OFFLINE.getStatus().toString().equals(
                res.getExtendInfo().get(DatabaseConstants.LINK_STATUS_KEY)));
        if (isAllChildResUnAuth) {
            clusterInstance.getExtendInfo()
                .put(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.OFFLINE.getStatus().toString());
        } else {
            clusterInstance.getExtendInfo()
                .put(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        }
        resources.add(clusterInstance);
        return resources;
    }

    /**
     * 注册的时候的check，有任意一个节点故障，都注册失败。
     *
     * @param environment mysql集群环境信息
     */
    @Override
    public void register(ProtectedEnvironment environment) {
        // 检验集群节点是否大于1，不管是哪种类型的集群，节点必须大于1
        List<ProtectedResource> agentList = environment.getDependencies().get(DatabaseConstants.AGENTS);
        if (agentList.size() <= 1) {
            throw new LegoCheckedException(DatabaseErrorCode.CLUSTER_NODE_NUMBER_ERROR,
                    "cluster node number error.");
        }

        String clusterType = environment.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE);
        if (MysqlConstants.EAPP.equals(clusterType) && agentList.size() > MysqlConstants.MAX_NODES_COUNT_FOR_EAPP) {
            throw new LegoCheckedException(MysqlErrorCode.OVER_LIMIT_OF_NODES,
                new String[] {MysqlConstants.MAX_NODES_COUNT_FOR_EAPP + ""}, "cluster node number error.");
        }

        // 检验是否有节点离线
        environment.getDependencies().get(DatabaseConstants.AGENTS).forEach(host -> {
            checkNodeOnline(environment, host);
        });

        // 校验集群环境是否存在主机已经被创建，避免重复或者交叉创建
        checkClusterIsExists(environment);

        // 设置集群信息，修改时候，uuid不为空，此时不需要构建UUID
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            environment.setUuid(UUIDGenerator.getUUID());
        }
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    private void checkNodeOnline(ProtectedEnvironment environment, ProtectedResource protectedResource) {
        ProtectedEnvironment env = mysqlBaseService.getEnvironmentById(protectedResource.getUuid());
        if (LinkStatusEnum.OFFLINE.getStatus().toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(env))) {
            throw new DataProtectionAccessException(CommonErrorCode.HOST_OFFLINE, new String[]{},
                    "Protected environment is offLine!");
        }
        environment.setEndpoint(env.getEndpoint());
    }

    private void checkClusterIsExists(ProtectedEnvironment environment) {
        // 获取新增环境依赖的Agent信息
        List<String> hostIds = environment.getDependencies()
                .get(DatabaseConstants.AGENTS)
                .stream()
                .map(ResourceBase::getUuid)
                .collect(Collectors.toList());

        // 获取数据库里已存在的mysql集群环境信息
        HashMap<String, Object> cons = new HashMap<>();
        cons.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.MYSQL_CLUSTER.getType());
        PageListResponse<ProtectedResource> clusterResult = resourceService.query(0, DatabaseConstants.PAGE_SIZE, cons);
        if (CollectionUtils.isEmpty(clusterResult.getRecords())) {
            return;
        }

        // 参数对比前，过滤掉修改情况需要忽略的mysql集群
        clusterResult.getRecords().stream()
                .filter(cluster -> !(Objects.equals(cluster.getUuid(), environment.getUuid())))
                .forEach(cluster -> {
            ProtectedEnvironment env = mysqlBaseService.getEnvironmentById(cluster.getUuid());
            env.getDependencies().get(DatabaseConstants.AGENTS).forEach(agent -> {
                if (hostIds.contains(agent.getUuid())) {
                    log.info("Host uuid:{} is already in mysql cluster uuid:{}!", agent.getUuid(), cluster.getUuid());
                    throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED,
                        "Host is already in mysql cluster!");
                }
            });
        });
    }

    /**
     * 集群环境监控检查的时候，有任意一个节点可用并且mysql服务正常，都算是集群在线
     * 抛出异常即可，框架自动去更新环境状态为离线
     *
     * @param environment 受保护资源
     */
    @Override
    public void validate(ProtectedEnvironment environment) {
        mysqlBaseService.healthCheckAllNodes(environment);
        String clusterType = environment.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE);
        ClusterInstanceOnlinePolicy policy = MysqlConstants.EAPP.equals(clusterType)
            ? ClusterInstanceOnlinePolicy.ALL_NODES_ONLINE
            : ClusterInstanceOnlinePolicy.ANY_NODE_ONLINE;
        instanceResourceService.healthCheckClusterInstanceOfEnvironment(environment, policy);
        List<ProtectedResource> agents = environment.getDependencies().get(DatabaseConstants.AGENTS);
        List<ProtectedResource> normalRes = agents.stream().filter(host -> {
            ProtectedEnvironment env = mysqlBaseService.getEnvironmentById(host.getUuid());
            return LinkStatusEnum.ONLINE.getStatus().toString()
                    .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(env));
        }).collect(Collectors.toList());
        if (MysqlConstants.EAPP.equals(clusterType)) {
            if (normalRes.size() != agents.size()) {
                throw new DataProtectionAccessException(CommonErrorCode.HOST_OFFLINE, new String[] {},
                    "Protected environment is offLine!");
            }
            return;
        }
        if (normalRes.isEmpty()) {
            throw new DataProtectionAccessException(CommonErrorCode.HOST_OFFLINE, new String[] {},
                "Protected environment is offLine!");
        }
    }
}
