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
package openbackup.redis.plugin.interceptor;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.database.base.plugin.utils.AuthParamUtil;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.redis.plugin.constant.RedisConstant;
import openbackup.redis.plugin.provider.RedisAgentProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * Redis数据库备份拦截器实现类
 *
 */
@Slf4j
@Component
public class RedisBackupInterceptor extends AbstractDbBackupInterceptor {
    private final ResourceService resourceService;

    private final KerberosService kerberosService;

    private final EncryptorService encryptorService;

    private final RedisAgentProvider redisAgentProvider;

    /**
     * redis备份拦截器构造方法
     *
     * @param resourceService 资源服务
     * @param kerberosService kerberosService
     * @param encryptorService encryptorService
     * @param redisAgentProvider redisAgentProvider
     */
    public RedisBackupInterceptor(ResourceService resourceService, KerberosService kerberosService,
        EncryptorService encryptorService, RedisAgentProvider redisAgentProvider) {
        this.resourceService = resourceService;
        this.kerberosService = kerberosService;
        this.encryptorService = encryptorService;
        this.redisAgentProvider = redisAgentProvider;
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        // PM 默认创建了data仓的StorageRepository对象, DME在收到data仓的时候，默认给创建了meta仓和data仓
        // 默认为单文件系统，多文件系统需要在高级参数中添加advanceParams.put("multiFileSystem", "true")
        // 目录副本格式1-INNER_DIRECTORY
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        Map<String, String> envExtendInfo = backupTask.getProtectEnv().getExtendInfo();
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SHARDING.getType());
        return backupTask;
    }

    @Override
    protected void supplyAgent(BackupTask backupTask) {
        // 获取集群ID
        String clusterId = backupTask.getProtectObject().getUuid();
        // 获取集群
        ProtectedResource clusterProtectedResource = resourceService.getResourceById(clusterId).get();
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(clusterProtectedResource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .build();

        backupTask.setAgents(redisAgentProvider.getSelectedAgents(agentSelectParam));
    }

    @Override
    protected void supplyNodes(BackupTask backupTask) {
        String clusterId = backupTask.getProtectObject().getUuid();
        ProtectedResource clusterProtectedResource = resourceService.getResourceById(clusterId).get();
        List<ProtectedResource> redisNodes = clusterProtectedResource.getDependencies().get(DatabaseConstants.CHILDREN);
        redisNodes.stream()
            .forEach(redisNode -> AuthParamUtil.convertKerberosAuth(redisNode.getAuth(), kerberosService,
                redisNode.getAuth().getExtendInfo().get(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID),
                encryptorService));
        List<TaskEnvironment> nodes = redisNodes.stream().map(this::toTaskEnvironment).collect(Collectors.toList());
        log.info("nodeList.size:{}", nodes.size());
        // 为每个节点添加agentId信息
        for (TaskEnvironment node : nodes) {
            ProtectedResource agent = redisNodes.stream()
                .filter(item -> item.getUuid().equals(node.getUuid()))
                .collect(Collectors.toList())
                .get(0)
                .getDependencies()
                .get(DatabaseConstants.AGENTS)
                .get(0);
            Map<String, String> extendInformation = node.getExtendInfo();
            extendInformation.put(RedisConstant.AGENT_ID, agent.getUuid());
            node.setExtendInfo(extendInformation);
            // 检查扫描任务是否已完成。
            if (StringUtils.isEmpty(node.getExtendInfo().get("pair"))) {
                throw new LegoCheckedException(CommonErrorCode.SCAN_JOB_NOT_COMPLETE, "scan job is not complete");
            }
        }
        backupTask.getProtectEnv().setNodes(nodes);
    }

    @Override
    protected void checkConnention(BackupTask backupTask) {
        log.info("check redis cluster connection");
        LinkStatusEnum linkStatus = LinkStatusEnum.getByStatus(
            Integer.valueOf(backupTask.getProtectEnv().getLinkStatus()));
        if (LinkStatusEnum.OFFLINE.equals(linkStatus)) {
            log.error("The redis: {} is offline, backup error.", backupTask.getProtectEnv().getUuid());
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "The target environment is offline");
        }
    }

    /**
     * 判断目标资源是否可以应用此拦截器
     *
     * @param subType 目标资源subType
     * @return 是否可以应用此拦截器
     */
    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.REDIS.getType().equals(subType);
    }

    private TaskEnvironment toTaskEnvironment(ProtectedResource protectedResource) {
        return JsonUtil.read(JsonUtil.json(protectedResource), TaskEnvironment.class);
    }
}