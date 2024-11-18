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

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DbClusterProvider;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.common.MysqlErrorCode;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.mysql.resources.access.enums.MysqlRoleEnum;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.host.HostRestApi;
import openbackup.system.base.sdk.host.model.Host;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * mysql集群实例注册provider
 *
 */
@Component
@Slf4j
public class MysqlClusterInstanceProvider implements ResourceProvider {
    private final ProviderManager providerManager;

    private final EncryptorService encryptorService;

    private final MysqlBaseService mysqlBaseService;

    private MysqlInstanceProvider mysqlInstanceProvider;

    private AgentUnifiedService agentUnifiedService;

    private HostRestApi hostRestApi;

    /**
     * DatabaseResourceProvider 构造器注入
     *
     * @param providerManager provider管理器，获取bean和过滤bean
     * @param pluginConfigManager 配置文件管理器
     * @param encryptorService 加密服务
     * @param mysqlBaseService base工具服务
     */
    public MysqlClusterInstanceProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        EncryptorService encryptorService, MysqlBaseService mysqlBaseService) {
        this.providerManager = providerManager;
        this.encryptorService = encryptorService;
        this.mysqlBaseService = mysqlBaseService;
    }

    @Autowired
    public void setAgentUnifiedService(AgentUnifiedService agentUnifiedService) {
        this.agentUnifiedService = agentUnifiedService;
    }

    @Autowired
    public void setHostRestApi(HostRestApi hostRestApi) {
        this.hostRestApi = hostRestApi;
    }

    @Autowired
    public void setMysqlInstanceProvider(MysqlInstanceProvider mysqlInstanceProvider) {
        this.mysqlInstanceProvider = mysqlInstanceProvider;
    }

    /**
     * 资源是否更新主机信息配置
     *
     * @return MySQL资源是否更新主机信息配置
     */
    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = ResourceProvider.super.getResourceFeature();
        // MySQL资源环境扫描不需要更新主机信息
        resourceFeature.setShouldUpdateDependencyHostInfoWhenScan(false);
        return resourceFeature;
    }

    @Override
    public void check(ProtectedResource resource) {
        ResourceProvider.super.check(resource);
        checkEappCluster(resource);
    }

    @Override
    public void updateCheck(ProtectedResource resource) {
        ResourceProvider.super.updateCheck(resource);
        checkEappCluster(resource);
    }

    private void checkEappCluster(ProtectedResource resource) {
        String clusterType = resource.getExtendInfo().get(DatabaseConstants.CLUSTER_TYPE);
        if (!MysqlConstants.EAPP.equals(clusterType)) {
            return;
        }
        String filterFiled = MysqlConstants.MYSQL + "_" + clusterType;
        log.info("Find cluster: {} instance check provider.", filterFiled);
        DbClusterProvider dbClusterProvider = providerManager.findProvider(DbClusterProvider.class, filterFiled);
        if (dbClusterProvider == null) {
            log.error("Has no provider for {}", clusterType);
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Has no provider");
        }
        dbClusterProvider.checkIsCluster(resource);
    }

    /**
     * 检查受保护资源创建条件是否成立
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        // 检查集群类型
        checkIsCluster(resource);

        // 设置集群path，保证副本复制不会出错
        setClusterInstancePath(resource);

        // 设置实例认证状态
        resource.getExtendInfo().put(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());

        // 设置version
        setClusterInstanceVersion(resource);

        // 设置集群的部署操作系统
        setClusterInstanceDeployOperatingSystem(resource);
    }

    /**
     * 设置集群实例的path，保证副本复制不会出错
     *
     * @param resource 集群实例资源
     */
    private void setClusterInstancePath(ProtectedResource resource) {
        List<ProtectedResource> subInstance = resource.getDependencies().get(DatabaseConstants.CHILDREN);
        String path = subInstance.stream().map(instance -> {
            String agentsUuid = instance.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid();
            ProtectedEnvironment environment = mysqlBaseService.getEnvironmentById(agentsUuid);
            return environment.getEndpoint();
        }).collect(Collectors.joining(","));
        resource.setPath(path);
    }

    /**
     * set集训实例的version
     *
     * @param resource 集群实例资源
     */
    private void setClusterInstanceVersion(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = Optional.ofNullable(resource.getDependencies())
            .orElse(new HashMap<>());
        List<ProtectedResource> instances = Optional.ofNullable(dependencies.get(DatabaseConstants.CHILDREN))
            .orElse(Collections.emptyList());
        Optional<String> version = Optional.ofNullable(instances.get(0).getVersion());
        resource.setVersion(version.get());
    }

    /**
     * set集群实例的部署操作系统
     *
     * @param resource 集群实例资源
     */
    private void setClusterInstanceDeployOperatingSystem(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = Optional.ofNullable(resource.getDependencies())
            .orElse(new HashMap<>());
        List<ProtectedResource> instances = Optional.ofNullable(dependencies.get(DatabaseConstants.CHILDREN))
            .orElse(Collections.emptyList());
        Optional<String> deployOperatingSystem = Optional.ofNullable(
            instances.get(0).getExtendInfo().get(MysqlConstants.DEPLOY_OPERATING_SYSTEM));
        resource.getExtendInfo().put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, deployOperatingSystem.get());
    }

    private void checkIsCluster(ProtectedResource resource) {
        String clusterType = resource.getExtendInfo().get(DatabaseConstants.CLUSTER_TYPE);
        // eapp集群已在check中校验过，不需要再次校验
        if (MysqlConstants.EAPP.equals(clusterType)) {
            return;
        }
        DbClusterProvider dbClusterProvider = getDbClusterProvider(resource);
        // 集群条件校验
        log.info("Cluster target check begin.");
        // 调用PXC、AP各自的校验逻辑
        dbClusterProvider.checkIsCluster(resource);

        // 公有检验逻辑：
        // 1、校验集群节点的至少有主节点
        checkClusterHasMasterRole(resource);
    }

    /**
     * 检验PXC或者AP 集群至少有一个主节点
     *
     * @param resource 集群实例
     */
    private void checkClusterHasMasterRole(ProtectedResource resource) {
        List<ProtectedResource> childrenInstance = resource.getDependencies().get(DatabaseConstants.CHILDREN);
        List<Boolean> roleList = childrenInstance.stream().map(this::checkRole).collect(Collectors.toList());
        DbClusterProvider dbClusterProvider = getDbClusterProvider(resource);
        dbClusterProvider.checkNodeRoleCondition(roleList);
        if (roleList.contains(Boolean.TRUE)) {
            return;
        }
        log.error("Check Cluster has at least one master node failed.");
        throw new LegoCheckedException(MysqlErrorCode.CHECK_CLUSTER_FAILED, "Check Cluster master node failed.");
    }

    private DbClusterProvider getDbClusterProvider(ProtectedResource resource) {
        String clusterType = resource.getExtendInfo().get(DatabaseConstants.CLUSTER_TYPE);
        String filterFiled = MysqlConstants.MYSQL + "_" + clusterType;
        log.info("Find cluster: {} instance check provider.", filterFiled);
        return providerManager.findProvider(DbClusterProvider.class, filterFiled);
    }

    private boolean checkRole(ProtectedResource instance) {
        // 当用户删除子实例，再添加时，此时无实例扩展信息，需要去agent查询
        if (VerifyUtil.isEmpty(instance.getUuid())) {
            Host host = hostRestApi.queryHostByID(instance.getExtendInfoByKey(DatabaseConstants.HOST_ID));
            ProtectedEnvironment environment = new ProtectedEnvironment();
            environment.setEndpoint(host.getIp());
            environment.setPort(Integer.valueOf(host.getPort()));

            AppEnvResponse appEnvResponse = agentUnifiedService.getClusterInfo(instance, environment);
            if (appEnvResponse == null || appEnvResponse.getExtendInfo() == null) {
                throw new DataProtectionAccessException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED, new String[] {},
                    "Get mysql cluster error.");
            }
            Map<String, String> envExtendInfo = appEnvResponse.getExtendInfo();
            // 如果role为空，则报错
            if (VerifyUtil.isEmpty(envExtendInfo.get(DatabaseConstants.IS_MASTER))) {
                throw new DataProtectionAccessException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED, new String[] {},
                    "Role is null.");
            }
            if (MysqlRoleEnum.MASTER.getRole().equals(envExtendInfo.get(DatabaseConstants.IS_MASTER))) {
                log.info("Check Cluster has at least one master node success.");
                return true;
            }
            return false;
        }
        if (MysqlRoleEnum.MASTER.getRole().equals(instance.getExtendInfoByKey(DatabaseConstants.ROLE))) {
            log.info("Check Cluster has at least one master node success.");
            return true;
        }
        return false;
    }

    /**
     * 检查受保护资源修改时连通性
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
        // 首次注册会调用update接口，此时不会传集群实例下的agent即Dependencies为空。
        if (resource.getDependencies() != null) {
            // 由于框架敏感信息脱敏，当用户不修改实例时，赋值密码
            setPwd(resource);

            // 校验联通性
            checkConnection(resource);

            // 设置集群path，保证副本复制不会出错
            setClusterInstancePath(resource);

            // 更新时，只针对PXC集群先加密再解密，AP集群不涉及解密操作
            encryptResource(resource);

            // 校验集群（修改包括新增节点或者修改原先节点信息）
            checkIsCluster(resource);

            // 针对用户从界面全部删除子实例再添加,此时无version和操作系统
            if (!checkUpdateOrDeleteResources(resource)) {
                // 设置version
                setClusterInstanceVersion(resource);

                // 设置集群的部署操作系统
                setClusterInstanceDeployOperatingSystem(resource);
            }
        }
        // 设置实例认证状态
        resource.getExtendInfo().put(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
    }

    /**
     * 检查用户在界面是直接点击修改还是全部删除资源后重新添加
     *
     * @param resource 集群实例资源
     * @return boolean true:全部删除集群实例  false:修改集群实例
     */
    private boolean checkUpdateOrDeleteResources(ProtectedResource resource) {
        List<ProtectedResource> resourceList = resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .filter(instance -> !VerifyUtil.isEmpty(instance.getUuid()))
            .collect(Collectors.toList());
        return resourceList.isEmpty();
    }

    /**
     * 设置密码。当用户不做任何修改时从数据库中查询出密码并set进实例中
     *
     * @param resource 集群实例资源
     */
    private void setPwd(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = Optional.ofNullable(resource.getDependencies())
            .orElse(new HashMap<>());
        List<ProtectedResource> resources = Optional.ofNullable(dependencies.get(DatabaseConstants.CHILDREN))
            .orElse(Collections.emptyList());
        resources.forEach(instance -> {
            if (VerifyUtil.isEmpty(instance.getAuth().getAuthPwd())) {
                ProtectedResource oldInstance = mysqlBaseService.getResource(instance.getUuid());
                instance.getAuth().setAuthPwd(oldInstance.getAuth().getAuthPwd());
            }
        });
    }

    /**
     * 针对PXC集群，加密集群实例下的子实例的密码和port,后续框架优化注册和修改集群实例逻辑可删除此方法
     *
     * @param resource 集群实例资源
     */
    private void encryptResource(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = Optional.ofNullable(resource.getDependencies())
            .orElse(new HashMap<>());
        List<ProtectedResource> resources = Optional.ofNullable(dependencies.get(DatabaseConstants.CHILDREN))
            .orElse(Collections.emptyList());
        if (MysqlConstants.PXC.equals(resource.getExtendInfo().get(DatabaseConstants.CLUSTER_TYPE))) {
            resources.forEach(instance -> {
                // 加密auth下的pwd
                instance.getAuth().setAuthPwd(encryptorService.encrypt(instance.getAuth().getAuthPwd()));
                // 加密auth下的instancePort
                Map<String, String> extendInfo = instance.getAuth().getExtendInfo();
                String instancePort = extendInfo.get(DatabaseConstants.INSTANCE_PORT);
                extendInfo.put(DatabaseConstants.INSTANCE_PORT, encryptorService.encrypt(instancePort));
            });
        }
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType().equals(object.getSubType());
    }

    private void checkConnection(ProtectedResource resource) {
        List<ProtectedResource> resources = resource.getDependencies().get(DatabaseConstants.CHILDREN);
        if (CollectionUtils.isEmpty(resources)) {
            log.error("The clusterInstance does not have subInstances");
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "subInstance can not be empty");
        }
        ResourceConnectionCheckProvider connectionCheckProvider = providerManager.findProvider(
            ResourceConnectionCheckProvider.class, resources.get(0));
        resources.forEach(connectionCheckProvider::checkConnection);
    }
}