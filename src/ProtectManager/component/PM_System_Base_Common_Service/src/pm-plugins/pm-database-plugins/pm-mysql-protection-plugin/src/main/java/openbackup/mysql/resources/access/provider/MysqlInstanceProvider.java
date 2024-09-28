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
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.common.MysqlErrorCode;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * mysql单实例注册provider
 *
 */
@Component
@Slf4j
public class MysqlInstanceProvider implements ResourceProvider {
    private static final String STATUS = "status";

    private static final String SERVICE_RUNNING = "0";

    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final MysqlBaseService mysqlBaseService;

    private InstanceResourceService instanceResourceService;

    private EncryptorService encryptorService;

    private final ProviderManager providerManager;

    /**
     * DatabaseResourceProvider 构造器注入
     *
     * @param providerManager provider管理器，获取bean和过滤bean
     * @param resourceService 资源服务接口，查询数据用
     * @param agentUnifiedService agentUnifiedService
     * @param mysqlBaseService mysqlBaseService
     */
    public MysqlInstanceProvider(ProviderManager providerManager, MysqlBaseService mysqlBaseService,
        ResourceService resourceService, AgentUnifiedService agentUnifiedService) {
        this.providerManager = providerManager;
        this.resourceService = resourceService;
        this.mysqlBaseService = mysqlBaseService;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Autowired
    public void setEncryptorService(EncryptorService encryptorService) {
        this.encryptorService = encryptorService;
    }

    @Autowired
    public void setInstanceResourceService(InstanceResourceService instanceResourceService) {
        this.instanceResourceService = instanceResourceService;
    }

    /**
     * 检查mysql实例资源联通行
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        checkConnect(resource);

        // 校验是否已经存在实例
        checkInstanceExist(resource);

        // 设置path信息，否则复制的时候会报错
        String envId = resource.getExtendInfo().get(DatabaseConstants.HOST_ID);
        ProtectedEnvironment environment = mysqlBaseService.getEnvironmentById(envId);
        resource.setPath(environment.getEndpoint());

        // 刷新子实例的集群状态
        refreshClusterInstance(resource, environment);

        // 设置实例认证状态
        resource.getExtendInfo().put(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
    }

    private void checkConnect(ProtectedResource resource) {
        // 校验联通性
        ResourceConnectionCheckProvider provider = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            resource);
        ResourceCheckContext context = provider.tryCheckConnection(resource);
        if (VerifyUtil.isEmpty(context.getActionResults())) {
            log.error("MySQL instance check connection result is empty. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "check connection result is empty.");
        }
        ActionResult actionResult = context.getActionResults().get(IsmNumberConstant.ZERO);
        if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
            log.error("MySQL instance check connection failed. name: {}", resource.getName());
            throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()), "check connection failed.");
        }
    }

    /**
     * 针对集群实例下面的子实例，刷新子实例的集群状态，得到子实例是集群节点，还是普通节点
     * 用于恢复时，根据节点信息来做恢复操作。一般来说底层会先恢复集群节点，普通节点由数据库集群自己来同步。
     *
     * @param resource 实例信息
     * @param environment 环境信息
     */
    public void refreshClusterInstance(ProtectedResource resource, ProtectedEnvironment environment) {
        // 调用Agent接口，获取集群状态信息
        AppEnvResponse appEnvResponse = agentUnifiedService.getClusterInfo(resource, environment);
        if (appEnvResponse == null || appEnvResponse.getExtendInfo() == null) {
            throw new DataProtectionAccessException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED, new String[] {},
                "Get mysql cluster error.");
        }
        // 如果mysql服务没起来，则不用更新role
        Map<String, String> envExtendInfo = appEnvResponse.getExtendInfo();
        if (!SERVICE_RUNNING.equals(envExtendInfo.get(STATUS))) {
            throw new DataProtectionAccessException(MysqlErrorCode.SERVICE_NOT_RUNNING, new String[] {},
                "Mysql is not running.");
        }

        // 如果role，dataDir是空，则报错
        if (VerifyUtil.isEmpty(envExtendInfo.get(DatabaseConstants.IS_MASTER)) || VerifyUtil.isEmpty(
            envExtendInfo.get(DatabaseConstants.DATA_DIR))) {
            throw new DataProtectionAccessException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED, new String[] {},
                "The role or data dir is null.");
        }
        setInstanceResource(resource, envExtendInfo, environment);
    }

    private void setInstanceResource(ProtectedResource resource, Map<String, String> envExtendInfo,
        ProtectedEnvironment environment) {
        Map<String, String> resExtendInfo = resource.getExtendInfo();
        resExtendInfo.put(DatabaseConstants.ROLE, envExtendInfo.get(DatabaseConstants.IS_MASTER));
        // dataDir需要加密存储
        resExtendInfo.put(DatabaseConstants.DATA_DIR,
            encryptorService.encrypt(envExtendInfo.get(DatabaseConstants.DATA_DIR)));
        log.info("Mysql refresh cluster instance active standby success. ip: {}, role: {}", environment.getEndpoint(),
            resExtendInfo.get(DatabaseConstants.ROLE));
        // eapp添加主节点信息
        resExtendInfo.put(MysqlConstants.MASTER_INFO, envExtendInfo.get(MysqlConstants.MASTER_INFO));

        // 设置 version
        String version = envExtendInfo.get(DatabaseConstants.VERSION);
        if (VerifyUtil.isEmpty(version)) {
            log.error("Get MySQL-instance {} Version failed!", resource.getUuid());
            throw new LegoCheckedException(CommonErrorCode.TARGET_CLUSTER_AUTH_FAILED, "Get version failed!");
        }
        resource.setVersion(version);

        // 设置部署的操作系统
        String deployOperatingSystem = envExtendInfo.get(MysqlConstants.DEPLOY_OPERATING_SYSTEM);
        if (VerifyUtil.isEmpty(deployOperatingSystem)) {
            log.error("Get MySQL-instance {} deployOperatingSystem failed!", resource.getUuid());
            throw new LegoCheckedException(MysqlErrorCode.CHECK_MYSQL_DEPLOYMENT_MODEL_FAILED,
                "Get MySQL deployOperatingSystem failed!");
        }
        resource.getExtendInfo().put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, deployOperatingSystem);

        // 设置单实例的启动服务名称
        String serviceName = envExtendInfo.get(MysqlConstants.MYSQL_SERVICE_NAME);
        if (VerifyUtil.isEmpty(serviceName)) {
            log.error("Get MySQL-instance {} service name failed!", resource.getUuid());
            throw new LegoCheckedException(MysqlErrorCode.CHECK_SERVICE_FAILED, "Get service failed!");
        }
        resource.getExtendInfo().put(MysqlConstants.MYSQL_SERVICE_NAME, serviceName);

        // 设置单实例的系统驱动服务的方式
        String systemServiceType = envExtendInfo.get(MysqlConstants.MYSQL_SYSTEM_SERVICE_TYPE);
        if (VerifyUtil.isEmpty(serviceName)) {
            log.error("Get MySQL-instance {} system service name failed! default is systemctl", resource.getUuid());
            systemServiceType = MysqlConstants.SYSTEM_CTL;
        }
        resource.getExtendInfo().put(MysqlConstants.MYSQL_SYSTEM_SERVICE_TYPE, systemServiceType);

        // 设置实例的binlog.index文件路径
        String binLogIndexPath = envExtendInfo.get(MysqlConstants.LOG_BIN_INDEX_PATH);
        if (VerifyUtil.isEmpty(binLogIndexPath)) {
            log.error("Get MySQL-instance {} log bin path failed!", resource.getUuid());
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                "Get MySQL bin log path failed!");
        }
        resource.getExtendInfo().put(MysqlConstants.LOG_BIN_INDEX_PATH, encryptorService.encrypt(binLogIndexPath));
    }

    /**
     * 检查受保护资源mysql实例修改联通性检查
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
        checkConnect(resource);

        // 刷新子实例的集群状态
        refreshClusterInstance(resource, mysqlBaseService.getAgentBySingleInstanceUuid(resource.getUuid()));
        // 设置实例认证状态
        resource.getExtendInfo().put(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
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

    /**
     * provider过滤器，过滤条件接口
     *
     * @param object object 受保护资源
     * @return boolean true 获取该bean false过滤掉该bean
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType().equals(object.getSubType());
    }

    private void checkInstanceExist(ProtectedResource resource) {
        String hostId = resource.getExtendInfo().get(DatabaseConstants.HOST_ID);
        String instancePort = resource.getAuth().getExtendInfo().get(DatabaseConstants.INSTANCE_PORT);
        Map<String, Object> cons = new HashMap<>();
        cons.put(DatabaseConstants.HOST_ID, hostId);
        cons.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType());
        cons.put(DatabaseConstants.INSTANCE_PORT, instancePort);
        List<ProtectedResource> records = resourceService.query(0, 1, cons).getRecords();
        if (!records.isEmpty()) {
            log.info("MySQL instance is already exist.");
            throw new DataProtectionAccessException(CommonErrorCode.DB_INSTANCE_HAS_REGISTERED, new String[] {},
                "MySQL instance is already exist.");
        }
    }

    @Override
    public void healthCheck(ProtectedResource resource) {
        instanceResourceService.healthCheckSingleInstance(resource);
    }
}
