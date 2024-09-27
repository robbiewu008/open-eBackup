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
package openbackup.obs.plugin.provider;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.obs.plugin.common.ObjectStorageCommonTool;
import openbackup.obs.plugin.common.constants.EnvironmentConstant;
import openbackup.obs.plugin.entity.BucketInfoEntity;
import openbackup.obs.plugin.service.ObjectStorageAgentService;
import com.huawei.oceanprotect.repository.service.LocalStorageService;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;

import com.fasterxml.jackson.core.type.TypeReference;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * OBS对象集合备份Provider
 *
 * @author l00370588
 * @since 2023-12-25
 */
@Slf4j
@Component
public class ObjectStorageBackupProvider implements BackupInterceptorProvider {
    private static final String KEY_SMALL_FILE_AGGREGATION = "aggregateSwitch";

    private static final String PERMANENT_INCREMENT = "foreverIncrementBackup";

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private ObjectStorageAgentService agentService;

    @Autowired
    private EncryptorService encryptorService;

    @Autowired
    private LocalStorageService localStorageService;

    @Autowired
    private DeployTypeService deployTypeService;

    @Override
    public BackupTask initialize(BackupTask backupTask) {
        if (backupTask.getProtectObject() == null) {
            log.error("ProtectObject is empty.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "ProtectObject is illegal.");
        }

        if (backupTask.getProtectEnv() == null) {
            log.error("env is empty.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "ProtectEnv is illegal.");
        }

        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);

        // 解密sk & 代理密码
        decryptData(backupTask);

        setStorageType(backupTask);

        // 校验连通性
        checkConnection(backupTask);

        Map<String, String> advanceParams = backupTask.getAdvanceParams();
        // 不开启永久增量备份且开启小文件聚合，副本格式设置为目录格式
        boolean isSmallFileAggregation = Boolean.parseBoolean(advanceParams.get(KEY_SMALL_FILE_AGGREGATION));
        if (!StringUtils.equals(backupTask.getBackupType(), PERMANENT_INCREMENT) && isSmallFileAggregation) {
            backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        }

        // 校验endpoint
        checkEndpoint(backupTask);

        checkAgents(backupTask);

        if (CollectionUtils.isNotEmpty(backupTask.getRepositories())) {
            String localEsn = "";
            if (!deployTypeService.isE1000()) {
                localEsn = localStorageService.getStorageInfo().getEsn();
            }
            ObjectStorageCommonTool.addRepositoryRole(backupTask.getRepositories(), localEsn);

            // 设置cache仓
            updateRepositories(backupTask, localEsn);
        }
        log.info("Object Backup taskId:{}, Repositories size {},", backupTask.getTaskId(),
            backupTask.getRepositories().size());
        return backupTask;
    }

    @Override
    public List<LockResourceBo> getLockResources(ProtectedResource resource) {
        log.info("start backup lock bucket resources, resource name is {}", resource.getName());
        String envId = resource.getRootUuid();
        String bucketListStr = Optional.ofNullable(resource.getExtendInfo())
            .map(extendInfo -> extendInfo.get(EnvironmentConstant.KEY_BUCKETLIST))
            .orElse("");
        List<LockResourceBo> lockList = new ArrayList<>();
        if (StringUtils.isNotEmpty(bucketListStr)) {
            List<BucketInfoEntity> buckets = JsonUtil.read(bucketListStr, new TypeReference<List<BucketInfoEntity>>() {
            });
            for (BucketInfoEntity buckInfo : buckets) {
                String buckId = buckInfo.getName() + "_" + envId;
                lockList.add(new LockResourceBo(buckId, LockType.READ));
                log.info("add read lock to bucket[{}] in backup, resource name is {}", buckId, resource.getName());
            }
        }
        return lockList;
    }

    private void setStorageType(BackupTask backupTask) {
        String storageType = backupTask.getProtectEnv().getExtendInfo().get(EnvironmentConstant.KEY_STORAGE_TYPE);
        backupTask.getProtectEnv().getAuth().getExtendInfo().put(EnvironmentConstant.KEY_STORAGE_TYPE, storageType);
    }

    private void checkAgents(BackupTask task) {
        String agents = task.getProtectEnv().getExtendInfo().get(EnvironmentConstant.KEY_AGENTS);
        List<Endpoint> objectStorageEndpoint = agentService.getObjectStorageEndpoint(agents);
        if (CollectionUtils.isNotEmpty(objectStorageEndpoint)) {
            task.setAgents(objectStorageEndpoint);
        }
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.OBJECT_SET.equalsSubType(subType);
    }

    private void decryptData(BackupTask backupTask) {
        // 解密sk
        String decryptedSk = encryptorService.decrypt(
            backupTask.getProtectEnv().getAuth().getExtendInfo().get(EnvironmentConstant.KEY_SK));
        backupTask.getProtectEnv().getAuth().getExtendInfo().put(EnvironmentConstant.KEY_SK, decryptedSk);

        // 开启代理需要解密代理密码
        if (isEnableProxy(backupTask)) {
            String decryptedPwd = encryptorService.decrypt(
                backupTask.getProtectEnv().getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_PWD));
            backupTask.getProtectEnv()
                .getAuth()
                .getExtendInfo()
                .put(EnvironmentConstant.KEY_PROXY_USER_PWD, decryptedPwd);
        }
    }

    private void checkEndpoint(BackupTask backupTask) {
        String endpoint = backupTask.getProtectEnv().getEndpoint();
        ObjectStorageCommonTool.checkEndPoint(endpoint);
        backupTask.getProtectEnv().getAuth().getExtendInfo().put(EnvironmentConstant.ENDPOINT, endpoint);
    }

    private boolean checkConnection(BackupTask backupTask) {
        String uuid = backupTask.getProtectEnv().getUuid();
        ProtectedResource protectedResource = resourceService.getBasicResourceById(uuid)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "resource not found."));
        // 任一连通即为连通
        ActionResult[] resultList = agentService.checkConnection(protectedResource);
        log.info("object storage checkConnection, resultList.length: {}", resultList.length);
        for (ActionResult result : resultList) {
            if (result.getCode() == ActionResult.SUCCESS_CODE) {
                return true;
            }
        }
        log.error("check connection failed, name: {}", protectedResource.getName());
        throw new LegoCheckedException(Long.parseLong(resultList[0].getBodyErr()), "check connection failed.");
    }

    private boolean isEnableProxy(BackupTask backupTask) {
        String proxyEnable = backupTask.getProtectEnv()
            .getAuth()
            .getExtendInfo()
            .get(EnvironmentConstant.KEY_PROXY_ENABLE);
        return StringUtils.equals(proxyEnable, EnvironmentConstant.PROXY_ENABLE_VALUE);
    }

    private void updateRepositories(BackupTask backupTask, String esn) {
        List<StorageRepository> repositories = backupTask.getRepositories();
        StorageRepository defaultRepository = repositories.get(IsmNumberConstant.ZERO);
        for (StorageRepository repository : repositories) {
            if (StringUtils.equals(repository.getId(), esn)) {
                defaultRepository = repository;
                break;
            }
        }

        // 对象备份任务默认添加Data仓库，缺少Cache仓库
        StorageRepository cacheRepository = BeanTools.copy(defaultRepository, StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
    }
}
