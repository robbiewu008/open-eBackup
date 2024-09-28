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

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.common.MysqlErrorCode;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.mysql.resources.access.enums.MysqlRoleEnum;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

/**
 * MySQl资源查询agent主机的provider
 *
 */
@Component
@Slf4j
public class MysqlAgentProvider extends DataBaseAgentSelector {
    private static final List<String> CLUSTER_TYPE_LIST = Arrays.asList(MysqlConstants.PXC, MysqlConstants.AP);

    private final MysqlBaseService mysqlBaseService;

    public MysqlAgentProvider(MysqlBaseService mysqlBaseService) {
        this.mysqlBaseService = mysqlBaseService;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        // 如果是数据库资源，根据数据库对应的单实例对应的Agent信息
        ProtectedResource resource = agentSelectParam.getResource();
        String subType = resource.getSubType();
        String backupType = agentSelectParam.getParameters().get(DatabaseConstants.BACKUP_TYPE_KEY);
        if (MysqlResourceSubTypeEnum.MYSQL_DATABASE.getType().equals(subType)) {
            return getAgentsByDatabase(resource.getParentUuid(), backupType);
        }

        // 获取集群实例或者是单实例的version，设置到保护对象的extendInfo里
        ProtectedResource clusterInstanceRes = mysqlBaseService.getResource(resource.getUuid());
        resource.setExtendInfo(
            mysqlBaseService.supplyExtendInfo(clusterInstanceRes.getVersion(), resource.getExtendInfo()));
        // 针对集群实例，设置Agents信息
        if (MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType().equals(subType)) {
            return getAgentsInClusterInstance(resource, backupType);
        }
        final List<Endpoint> endpoints = super.getSelectedAgents(agentSelectParam);
        log.info("In single res agent size: {}", endpoints.size());
        return endpoints;
    }

    private List<Endpoint> getAgentsByDatabase(String resourceParentUuid, String backupType) {
        // 拦截数据库恢复（日志备份只能在主节点进行备份）
        checkDatabaseBackup(resourceParentUuid, backupType);
        // 获取单实例对应的Agent信息
        ProtectedEnvironment agentEnv = mysqlBaseService.getAgentBySingleInstanceUuid(resourceParentUuid);
        log.info("Mysql database agent uuid: {}, agent ip: {}", agentEnv.getUuid(), agentEnv.getEndpoint());
        return Collections.singletonList(mysqlBaseService.getAgentEndpoint(agentEnv));
    }

    private void checkDatabaseBackup(String parentUuid, String backupType) {
        if (!DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType)) {
            return;
        }
        ProtectedResource instanceResource = mysqlBaseService.getResource(parentUuid);
        Map<String, String> extendInfo = instanceResource.getExtendInfo();
        if (!CLUSTER_TYPE_LIST.contains(extendInfo.get(DatabaseConstants.CLUSTER_TYPE))) {
            return;
        }
        if (MysqlRoleEnum.SLAVE.getRole().equals(extendInfo.get(DatabaseConstants.ROLE))) {
            throw new LegoCheckedException(MysqlErrorCode.CHECK_CLUSTER_DATABASE_BACKUP_FAILED,
                "This log backup job in slave node not execute.");
        }
    }

    private List<Endpoint> getAgentsInClusterInstance(ProtectedResource resource, String backupType) {
        // 从dependency里，获取集群实例下面的所有子实例
        List<ProtectedResource> singleInstanceResources =
            mysqlBaseService.getSingleInstanceByClusterInstance(resource.getUuid());
        log.info("In cluster, single ins size: {}", singleInstanceResources.size());
        List<Endpoint> agents = new ArrayList<>();
        // 遍历子实例信息
        for (ProtectedResource singleInstanceResource : singleInstanceResources) {
            log.info("Find mysql database agent. single ins res uuid: {}", singleInstanceResource.getUuid());
            // 如果是日志备份，则只能在主节点备份
            if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType)) {
                final String role = singleInstanceResource.getExtendInfo().get(DatabaseConstants.ROLE);
                if (!(MysqlRoleEnum.MASTER.getRole().equals(role))) {
                    continue;
                }
            }
            // 从子实例的dependency里，获取子实例对应的Agent主机
            ProtectedEnvironment agentEnv =
                mysqlBaseService.getAgentBySingleInstanceUuid(singleInstanceResource.getUuid());
            // 将Agent信息，放置到备份对象中
            log.info("Mysql database agent uuid: {}, agent ip: {}", agentEnv.getUuid(), agentEnv.getEndpoint());
            agents.add(mysqlBaseService.getAgentEndpoint(agentEnv));
        }
        log.info("Mysql agent size: {} in cluster.", agents.size());
        return agents;
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return MysqlResourceSubTypeEnum.isBelongToMysql(agentSelectParam.getResource().getSubType());
    }
}
