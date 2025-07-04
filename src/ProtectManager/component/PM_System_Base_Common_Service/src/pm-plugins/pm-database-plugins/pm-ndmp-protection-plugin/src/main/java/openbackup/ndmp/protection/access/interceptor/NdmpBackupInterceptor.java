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
package openbackup.ndmp.protection.access.interceptor;

import com.huawei.oceanprotect.repository.service.LocalStorageService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.ndmp.protection.access.constant.NdmpConstant;
import openbackup.ndmp.protection.access.service.NdmpService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import org.apache.commons.collections.MapUtils;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * NDMP备份资源接入
 *
 */

@Component
@Slf4j
public class NdmpBackupInterceptor extends AbstractDbBackupInterceptor {
    private final LocalStorageService localStorageService;

    private final NdmpService ndmpService;

    private final DeployTypeService deployTypeService;

    /**
     * 构造器
     *
     * @param localStorageService localStorageService
     * @param ndmpService ndmpService
     * @param deployTypeService deployTypeService
     */
    public NdmpBackupInterceptor(LocalStorageService localStorageService, NdmpService ndmpService,
        DeployTypeService deployTypeService) {
        this.localStorageService = localStorageService;
        this.ndmpService = ndmpService;
        this.deployTypeService = deployTypeService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.NDMP_BACKUPSET.getType().equals(object);
    }

    /**
     * 设置注册的agent信息到备份任务
     *
     * @param backupTask backupTask
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        String parentUuid = backupTask.getProtectObject().getRootUuid();
        log.info("NdmpBackupInterceptor supplyAgent, parentUuid:{}", parentUuid);
        String agents = backupTask.getAdvanceParams().get("agents");
        log.info("NdmpBackupInterceptor supplyAgent, agents:{}", agents);
        // 判断是否是文件系统
        String isFs = backupTask.getProtectObject().getExtendInfo().get(NdmpConstant.IS_FILE_SYSTEM);
        if (NdmpConstant.DIR.equals(isFs)) {
            // rootUuid是保护环境的id
            backupTask.getProtectObject().setParentUuid(backupTask.getProtectObject().getRootUuid());
        }
        backupTask.setAgents(ndmpService.getAgents(parentUuid, agents));
    }

    /**
     * 填充node信息
     *
     * @param backupTask backupTask
     */
    @Override
    protected void supplyNodes(BackupTask backupTask) {
        List<TaskEnvironment> taskEnvironments = ndmpService.supplyNodes();
        backupTask.getProtectEnv().setNodes(taskEnvironments);
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        TaskResource protectObject = backupTask.getProtectObject();
        Map<String, String> protectObjectExtendInfo = protectObject.getExtendInfo();
        String fullName = MapUtils.getString(protectObjectExtendInfo, "fullName");
        TaskResource taskResource = backupTask.getProtectObject();
        taskResource.setName(fullName);
        StorageRepository storageRepository = backupTask.getRepositories().get(0);
        Map<String, Object> extendInfo = Optional.ofNullable(storageRepository.getExtendInfo()).orElse(new HashMap<>());
        if (!deployTypeService.isE1000()) {
            storageRepository.setId(localStorageService.getStorageInfo().getEsn());
            extendInfo.put(NdmpConstant.REPOSITORIES_KEY_ENS, localStorageService.getStorageInfo().getEsn());
        }
        storageRepository.setRole(NdmpConstant.MASTER_ROLE);
        storageRepository.setExtendInfo(extendInfo);

        // 更新环境中的信息
        ndmpService.modifyBackupTaskParam(backupTask);
        log.info("end to modify backTask");
        return backupTask;
    }
}
