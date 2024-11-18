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
package openbackup.mysql.resources.access.interceptor;

import static openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE;
import static openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum.MYSQL_DATABASE;
import static openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum.isBelongToMysql;
import static openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceReq;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.database.base.plugin.utils.DatabaseScannerUtils;
import openbackup.database.base.plugin.utils.ProtectionTaskUtils;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.common.MysqlErrorCode;
import openbackup.mysql.resources.access.enums.MysqlRoleEnum;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections4.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.OptionalInt;
import java.util.stream.Collectors;

/**
 * MySQL数据库备份拦截器实现类
 *
 */
@Slf4j
@Component
public class MysqlDbBackupInterceptor extends AbstractDbBackupInterceptor {
    private static final String STATUS = "status";

    private static final String SERVICE_RUNNING = "0";

    private static final List<String> CLUSTER_TYPE_LIST = Arrays.asList(MysqlConstants.PXC, MysqlConstants.AP);

    /**
     * 日志备份类型
     */
    private static final String LOG_BACKUP_TYPE = "logBackup";

    private final MysqlBaseService mysqlBaseService;

    private final AgentUnifiedService agentUnifiedService;

    @Autowired
    private ResourceService resourceService;

    /**
     * mysql备份拦截器构造方法
     *
     * @param mysqlBaseService mysql应用基本的Service
     * @param agentUnifiedService agentUnifiedService
     */
    public MysqlDbBackupInterceptor(MysqlBaseService mysqlBaseService, AgentUnifiedService agentUnifiedService) {
        this.agentUnifiedService = agentUnifiedService;
        this.mysqlBaseService = mysqlBaseService;
    }

    /**
     * mysql supply Nodes
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void supplyNodes(BackupTask backupTask) {
        super.supplyNodes(backupTask);
        String subType = backupTask.getProtectObject().getSubType();
        if (MYSQL_CLUSTER_INSTANCE.getType().equals(subType)) {
            List<TaskEnvironment> nodes = backupTask.getProtectEnv().getNodes();
            List<ProtectedResource> singleInstanceResources = mysqlBaseService.getSingleInstanceByClusterInstance(
                backupTask.getProtectObject().getUuid());
            mysqlBaseService.setNodesAuth(nodes, singleInstanceResources);
        } else if (MYSQL_DATABASE.getType().equals(subType)) {
            ProtectedResource parentResource = mysqlBaseService.getResource(
                backupTask.getProtectObject().getParentUuid());
            if (backupTask.getProtectObject().getExtendInfo() == null) {
                backupTask.getProtectObject().setExtendInfo(new HashMap<>());
            }
            backupTask.getProtectObject()
                .getExtendInfo()
                .put(MysqlConstants.INSTANCE_IP, parentResource.getExtendInfoByKey(MysqlConstants.INSTANCE_IP));
            if (parentResource.getExtendInfoByKey(DatabaseConstants.CHARSET) != null) {
                backupTask.getProtectObject()
                    .getExtendInfo()
                    .put(DatabaseConstants.CHARSET, parentResource.getExtendInfoByKey(DatabaseConstants.CHARSET));
            }
            if (parentResource.getExtendInfoByKey(MysqlConstants.MY_CNF_PATH) != null) {
                backupTask.getProtectObject()
                    .getExtendInfo()
                    .put(MysqlConstants.MY_CNF_PATH, parentResource.getExtendInfoByKey(MysqlConstants.MY_CNF_PATH));
            }
        } else {
            log.info("Mysql single instance backup. no need supply nodes in plugin.");
        }
    }

    /**
     * MySQL数据库拦截器操作实现
     * <p>
     * 1. 如果是日志备份，则新增日志仓库，移除数据仓库；不管是哪种备份，都需要新增cache仓
     * 2. 设置备份任务拆分时所需要的部署类型
     *
     * @param backupTask 初始的备份对象
     * @return 经过应用拦截器后的备份对象
     */
    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        updateRepositories(backupTask);
        Map<String, String> envExtendInfo = backupTask.getProtectEnv().getExtendInfo();
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        String subType = backupTask.getProtectObject().getSubType();
        if (MYSQL_CLUSTER_INSTANCE.getType().equals(subType) && MysqlConstants.EAPP.equals(
            envExtendInfo.get(DatabaseConstants.CLUSTER_TYPE))) {
            envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SHARDING.getType());
            if (DatabaseConstants.LOG_BACKUP_TYPE.equalsIgnoreCase(backupTask.getBackupType())) {
                throw new LegoCheckedException(MysqlErrorCode.EAPP_MYSQL_NOT_SUPPORT_LOG_BACKUP,
                    "Log backup for eappmysql is unsupported.");
            }
        }
        // 针对数据库备份，设置auth信息
        if (MYSQL_DATABASE.getType().equals(subType)) {
            ProtectedResource resource = mysqlBaseService.getResource(backupTask.getProtectObject().getParentUuid());
            backupTask.getProtectObject().setAuth(resource.getAuth());
        }

        // 设置速度统计方式为UBC
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);

        // 设置副本格式
        ProtectionTaskUtils.setCopyFormat(backupTask);
        return backupTask;
    }

    /**
     * 判断目标资源是否可以应用此拦截器
     *
     * @param subType 目标资源subType
     * @return 是否可以应用此拦截器
     */
    @Override
    public boolean applicable(String subType) {
        return Objects.nonNull(subType) && isBelongToMysql(subType);
    }

    /**
     * 根据备份任务类型的不同，更新仓库
     * 如果是日志备份，则新增日志仓库，移除数据仓库；不管是哪种备份，都需要新增cache仓
     *
     * @param backupTask 通用备份框架备份参数对象
     */
    private void updateRepositories(BackupTask backupTask) {
        List<StorageRepository> repositories = backupTask.getRepositories();
        if (LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            backupTask.addRepository(logRepository);
            repositories.remove(0);
        }
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        backupTask.setRepositories(repositories);
    }

    /**
     * 如果是数据库粒度的资源备份，需要根据数据库的parentUuid找到对应的单实例资源，
     * 然后再根据单实例资源的parentUuid，找到对应的主机
     * 然后根据Agent主机，拿到Agent信息
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        // 如果是数据库资源，根据数据库对应的单实例对应的Agent信息，设置到备份对象中
        String parentUuid = backupTask.getProtectObject().getParentUuid();
        String subType = backupTask.getProtectObject().getSubType();
        if (MYSQL_DATABASE.getType().equals(subType)) {
            backupTask.setAgents(new ArrayList<>());
            // 拦截数据库恢复（日志备份只能在主节点进行备份）
            checkDatabaseBackup(parentUuid, backupTask);
            // 获取单实例对应的Agent信息
            ProtectedEnvironment agentEnv = mysqlBaseService.getAgentBySingleInstanceUuid(parentUuid);
            // 将Agent信息，放置到备份对象中
            addAgents(backupTask, mysqlBaseService.getAgentEndpoint(agentEnv));
            return;
        }

        // 获取集群实例或者是单实例的version，设置到保护对象的extendInfo里
        ProtectedResource clusterInstanceRes = mysqlBaseService.getResource(backupTask.getProtectObject().getUuid());
        backupTask.getProtectObject()
            .setExtendInfo(mysqlBaseService.supplyExtendInfo(clusterInstanceRes.getVersion(),
                backupTask.getProtectObject().getExtendInfo()));

        // 针对集群实例，设置Agents信息
        if (MYSQL_CLUSTER_INSTANCE.getType().equals(subType)) {
            setAgentsInClusterInstance(backupTask);
            return;
        }
        super.supplyAgent(backupTask);
    }

    private void checkDatabaseBackup(String parentUuid, BackupTask backupTask) {
        if (!LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            return;
        }
        ProtectedResource parentResource = mysqlBaseService.getResource(parentUuid);
        Map<String, String> extendInfo = parentResource.getExtendInfo();
        // 单实例放行
        if (!CLUSTER_TYPE_LIST.contains(extendInfo.get(DatabaseConstants.CLUSTER_TYPE))) {
            return;
        }
        // 集群实例检测节点角色备节点抛出错误
        if (MysqlRoleEnum.SLAVE.getRole().equals(extendInfo.get(DatabaseConstants.ROLE))) {
            throw new LegoCheckedException(MysqlErrorCode.CHECK_CLUSTER_DATABASE_BACKUP_FAILED,
                "Check the database of instance backup failed");
        }
    }

    private void setAgentsInClusterInstance(BackupTask backupTask) {
        backupTask.setAgents(new ArrayList<>());
        // 从dependency里，获取集群实例下面的所有子实例
        List<ProtectedResource> singleInstanceResources = mysqlBaseService.getSingleInstanceByClusterInstance(
            backupTask.getProtectObject().getUuid());
        // 遍历子实例信息
        for (ProtectedResource singleInstanceResource : singleInstanceResources) {
            // 如果是日志备份，则只能在主节点备份
            if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
                final String role = singleInstanceResource.getExtendInfo().get(DatabaseConstants.ROLE);
                if (!(MysqlRoleEnum.MASTER.getRole().equals(role))) {
                    continue;
                }
            }
            // 从子实例的dependency里，获取子实例对应的Agent主机
            ProtectedEnvironment agentEnv = mysqlBaseService.getAgentBySingleInstanceUuid(
                singleInstanceResource.getUuid());
            // 将Agent信息，放置到备份对象中
            addAgents(backupTask, mysqlBaseService.getAgentEndpoint(agentEnv));
        }
    }

    /**
     * 往备份对象里，添加endpoint信息
     *
     * @param backupTask 备份对象
     * @param endpoint Agent对应的endpoint对象
     */
    private void addAgents(BackupTask backupTask, Endpoint endpoint) {
        List<Endpoint> agents = backupTask.getAgents();
        if (agents == null) {
            List<Endpoint> endpoints = new ArrayList<>();
            endpoints.add(endpoint);
            backupTask.setAgents(endpoints);
        } else {
            backupTask.getAgents().add(endpoint);
        }
    }

    /**
     * 检查连通性
     *
     * @param backupTask backupTask
     */
    @Override
    protected void checkConnention(BackupTask backupTask) {
        if (!MYSQL_CLUSTER_INSTANCE.getType().equals(backupTask.getProtectObject().getSubType())) {
            super.checkConnention(backupTask);
            if (MYSQL_DATABASE.getType().equals(backupTask.getProtectObject().getSubType())) {
                checkDatabaseExists(backupTask);
            }
            return;
        }
        // 针对集群实例备份的情况，对agents里每一个节点做mysql服务的连通性检查
        // 只要有一个可用的，就可以把备份任务下发下去，并且需要移除不可用的agent节点
        String clusterType = backupTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.CLUSTER_TYPE);
        // eapp需要始终向所有节点下发任务
        if (MysqlConstants.EAPP.equals(clusterType)) {
            return;
        }
        List<ProtectedResource> singleInstanceResources = mysqlBaseService.getSingleInstanceByClusterInstance(
            backupTask.getProtectObject().getUuid());
        for (ProtectedResource singleInstanceResource : singleInstanceResources) {
            ProtectedEnvironment agentEnv = mysqlBaseService.getAgentBySingleInstanceUuid(
                singleInstanceResource.getUuid());
            try {
                AppEnvResponse appEnvResponse = agentUnifiedService.getClusterInfo(singleInstanceResource, agentEnv);
                if (appEnvResponse == null || appEnvResponse.getExtendInfo() == null || !SERVICE_RUNNING.equals(
                    appEnvResponse.getExtendInfo().get(STATUS))) {
                    backupTask.getAgents().removeIf(endpoint -> agentEnv.getEndpoint().equals(endpoint.getIp()));
                }
            } catch (LegoCheckedException | DataProtectionAccessException e) {
                log.error("check mysql instance error.", ExceptionUtil.getErrorMessage(e));
                backupTask.getAgents().removeIf(endpoint -> agentEnv.getEndpoint().equals(endpoint.getIp()));
            }
        }
        if (backupTask.getAgents().isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "All agent network error");
        }
        log.info("mysql cluster instance available agents is: {}",
            backupTask.getAgents().stream().map(Endpoint::getIp).collect(Collectors.joining(",", "[", "]")));
    }

    private void checkDatabaseExists(BackupTask backupTask) {
        TaskEnvironment protectEnv = backupTask.getProtectEnv();
        List<ProtectedResource> instances = DatabaseScannerUtils.getInstancesByEnvironment(protectEnv.getUuid(),
            MYSQL_SINGLE_INSTANCE.getType(), resourceService);
        Optional<ProtectedResource> instanceOp = instances.stream()
            .filter(protectedResource -> StringUtils.equals(protectedResource.getUuid(),
                backupTask.getProtectObject().getParentUuid()))
            .findFirst();
        if (!instanceOp.isPresent()) {
            log.info("instance not exist");
            return;
        }
        ListResourceReq mysqlDbsRequest = new ListResourceReq();
        mysqlDbsRequest.setAppEnv(BeanTools.copy(protectEnv, AppEnv::new));
        mysqlDbsRequest.setApplication(BeanTools.copy(instanceOp.get(), Application::new));
        AgentDetailDto detail = agentUnifiedService.getDetail(MYSQL_SINGLE_INSTANCE.getType(),
            backupTask.getProtectEnv().getEndpoint(), backupTask.getProtectEnv().getPort(), mysqlDbsRequest);
        List<AppResource> resourceList = detail.getResourceList();
        if (CollectionUtils.isEmpty(resourceList)) {
            log.error("resourceList is empty");
            throw new LegoCheckedException(DatabaseErrorCode.DATABASE_NOT_EXISTS, new String[] {
                backupTask.getProtectObject().getName(), backupTask.getProtectObject().getParentName()
            }, "database not exists.");
        }
        boolean isContains = resourceList.stream()
            .map(AppResource::getName)
            .anyMatch(s -> StringUtils.equals(s, backupTask.getProtectObject().getName()));
        if (!isContains) {
            throw new LegoCheckedException(DatabaseErrorCode.DATABASE_NOT_EXISTS, new String[] {
                backupTask.getProtectObject().getName(), backupTask.getProtectObject().getParentName()
            }, "database not exists.");
        }
    }

    /**
     * 副本格式
     *
     * @param backupTask backupTask
     * @return 副本格式
     */
    @Override
    protected OptionalInt obtainFormat(BackupTask backupTask) {
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            return OptionalInt.of(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        } else {
            return OptionalInt.of(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        }
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return true;
    }
}