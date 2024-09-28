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
package openbackup.informix.protection.access.provider.back;

import static openbackup.database.base.plugin.common.DatabaseConstants.CHILDREN;
import static openbackup.informix.protection.access.constant.InformixConstant.HOST_ID;
import static openbackup.informix.protection.access.constant.InformixConstant.MASTER_STATUS_LIST;

import openbackup.access.framework.resource.persistence.dao.ProtectedEnvironmentExtendInfoMapper;
import com.huawei.oceanprotect.client.resource.manager.service.AgentLanFreeService;
import com.huawei.oceanprotect.client.resource.manager.service.dto.AgentLanFreeAixViewDTO;
import com.huawei.oceanprotect.client.resource.manager.utils.LanFreeValidator;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.informix.protection.access.constant.InformixConstant;
import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import io.jsonwebtoken.lang.Collections;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * informix备份拦截器
 *
 */
@Slf4j
@Component
public class InformixBackupInterceptorProvider extends AbstractDbBackupInterceptor {
    private final InformixService informixService;
    private final AgentLanFreeService agentLanFreeService;
    private final ProtectedEnvironmentExtendInfoMapper protectedEnvironmentExtendInfoMapper;
    private final ResourceService resourceService;

    public InformixBackupInterceptorProvider(InformixService informixService, AgentLanFreeService agentLanFreeService,
                ProtectedEnvironmentExtendInfoMapper protectedEnvironmentExtendInfoMapper,
                ResourceService resourceService) {
        this.informixService = informixService;
        this.agentLanFreeService = agentLanFreeService;
        this.protectedEnvironmentExtendInfoMapper = protectedEnvironmentExtendInfoMapper;
        this.resourceService = resourceService;
    }

    /**
     * provider过滤器，过滤条件接口
     *
     * @param resourceSubType 资源类型
     * @return boolean true 获取该bean false过滤掉该bean
     */
    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType().equals(resourceSubType)
                || ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType().equals(resourceSubType);
    }

    /**
     * intercept
     *
     * @param backupTask 备份任务参数对象{@link BackupTask}
     * @return BackupTask
     */
    @Override
    public BackupTask initialize(BackupTask backupTask) {
        log.info("Start to Backup task. task id :{}, backup type : {}",
                backupTask.getTaskId(), backupTask.getBackupType());
        // 设置保护环境扩展参数deployType
        setDeployType(backupTask);
        // 设置副本格式
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        // 设置速度统计方式为UBC
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);
        // 设置存储仓
        backupTask.setRepositories(getRepositories(backupTask));
        // 获取资源信息
        ProtectedResource resource = informixService.getResourceById(backupTask.getProtectObject().getUuid());
        // 设置副本大小
        setProtectObjectDataSize(backupTask, resource);
        // 设置nodes
        List<TaskEnvironment> envNodesByInstanceResource = informixService.getEnvNodesByInstanceResource(resource);
        backupTask.getProtectEnv().setNodes(envNodesByInstanceResource);
        // 设置agents
        List<Endpoint> agentsByInstanceResource = informixService.getAgentsByInstanceResource(resource);
        backupTask.setAgents(agentsByInstanceResource);
        return backupTask;
    }

    private void setProtectObjectDataSize(BackupTask backupTask, ProtectedResource resource) {
        log.info("Enter set copy size method. task id : {}", backupTask.getTaskId());
        Map<String, String> extendInfo = backupTask.getProtectObject().getExtendInfo();
        if (ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType().equals(resource.getSubType())) {
            setDataSize(backupTask, resource, extendInfo);
            return;
        }

        List<ProtectedResource> resourceList = resource.getDependencies().get(CHILDREN);
        for (ProtectedResource childResource : resourceList) {
            if (MASTER_STATUS_LIST.contains(childResource.getExtendInfo().get(InformixConstant.INSTANCESTATUS))) {
                setDataSize(backupTask, childResource, extendInfo);
                break;
            }
        }
    }

    private void setDataSize(BackupTask backupTask, ProtectedResource resource, Map<String, String> extendInfo) {
        ActionResult[] check = resourceService.check(resource);
        ActionResult actionResult = check[0];
        String dataSize = actionResult.getMessage();
        if (checkDataSize(backupTask, dataSize)) {
            log.error("Get data size success. task id : {}. data size : {}", backupTask.getTaskId(), dataSize);
            extendInfo.put(InformixConstant.DATA_SIZE, dataSize);
        }
    }

    private boolean checkDataSize(BackupTask backupTask, String dataSize) {
        try {
            Long.parseLong(dataSize);
        } catch (NumberFormatException exception) {
            log.error("Data size is invalid. task id : {}. data size : {}", backupTask.getTaskId(), dataSize);
            return false;
        }
        return true;
    }

    private List<StorageRepository> getRepositories(BackupTask backupTask) {
        String dbResourceId = backupTask.getProtectObject().getUuid();
        ProtectedResource dbResource = informixService.getResourceById(dbResourceId);
        String enableLogBakStr = dbResource.getExtendInfoByKey(InformixConstant.LOG_BACKUP);
        List<StorageRepository> repositories = backupTask.getRepositories();
        if (InformixConstant.LOG_BACKUP_OFF.equals(enableLogBakStr)) {
            if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
                log.error("The informix instance does not enable log backup, can not execute log backup. "
                        + "task id : {}, database uuid : {}.", backupTask.getTaskId(), dbResource.getUuid());
                throw new LegoCheckedException(CommonErrorCode.ERROR_EXIST_EXPORTING_LOG,
                        "The informix instance does not enable log backup.");
            }
        } else {
            StorageRepository logRepository =
                    BeanTools.copy(repositories.get(IsmNumberConstant.ZERO), StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            HashMap<String, Object> extendInfoMap = new HashMap<>(logRepository.getExtendInfo());
            extendInfoMap.put(InformixConstant.PERSISTENT_MOUNT, true);
            setManualMount(backupTask, dbResource, extendInfoMap);
            logRepository.setExtendInfo(extendInfoMap);
            repositories.add(logRepository);
        }

        StorageRepository repo = BeanTools.copy(repositories.get(IsmNumberConstant.ZERO), StorageRepository::new);
        repo.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(repo);

        // For log backup, the data repository and metadata repository do not need to be delivered.
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            return repositories
                    .stream()
                    .filter(storageRepository ->
                            (storageRepository.getType() != RepositoryTypeEnum.META.getType()
                            && storageRepository.getType() != RepositoryTypeEnum.DATA.getType()))
                    .collect(Collectors.toList());
        }
        return repositories;
    }

    private void setDeployType(BackupTask backupTask) {
        String subType = backupTask.getProtectObject().getSubType();
        String deployType = ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.equalsSubType(subType)
                ? DatabaseDeployTypeEnum.SINGLE.getType()
                : DatabaseDeployTypeEnum.AP.getType();
        Map<String, String> extendInfo = Optional.ofNullable(backupTask.getProtectEnv().getExtendInfo())
                .orElseGet(HashMap::new);
        extendInfo.put(DatabaseConstants.DEPLOY_TYPE, deployType);
        backupTask.getProtectEnv().setExtendInfo(extendInfo);
    }

    private void setManualMount(BackupTask backupTask,
                ProtectedResource dbResource, HashMap<String, Object> extendInfoMap) {
        log.info("Set manual mount, task id : {}, instance type : {}", backupTask.getTaskId(), dbResource.getSubType());
        if (!ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType().equals(dbResource.getSubType())) {
            return;
        }
        // 如果主机挂载了san client，插件需要手动挂载
        String serviceId = dbResource.getRootUuid();
        ProtectedResource serviceResource = informixService.getResourceById(serviceId);
        String hostId = serviceResource.getExtendInfoByKey(HOST_ID);
        if (shouldSetManualMount(backupTask, hostId)) {
            log.info("San client is configured on the current aix host. task id : {}, host id: {}",
                    backupTask.getTaskId(), hostId);
            extendInfoMap.put(InformixConstant.MANUAL_MOUNT, "true");
        }
    }

    private boolean shouldSetManualMount(BackupTask backupTask, String hostId) {
        if (!LanFreeValidator.checkIfAixHost(protectedEnvironmentExtendInfoMapper, hostId)) {
            log.info("The type of current host is not aix, task id : {}, host id : {}.",
                    backupTask.getTaskId(), hostId);
            return false;
        }

        AgentLanFreeAixViewDTO agentLanFreeAixViewDTO = new AgentLanFreeAixViewDTO();
        try {
            agentLanFreeAixViewDTO = agentLanFreeService.queryAixLanFree(hostId);
        } catch (LegoCheckedException exception) {
            log.error("Query aix lan free failed. task id : {}, error message : {}",
                    backupTask.getTaskId(), ExceptionUtil.getErrorMessage(exception));
        }
        List<String> sanclientResourceIds = agentLanFreeAixViewDTO.getSanclientResourceIds();
        return !Collections.isEmpty(sanclientResourceIds);
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return true;
    }
}
