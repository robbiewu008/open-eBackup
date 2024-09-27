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
package openbackup.data.access.framework.protection.service.archive;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import openbackup.data.access.client.sdk.api.framework.archive.ArchiveUnifiedRestApi;
import openbackup.data.access.framework.copy.mng.constant.CopyResourcePropertiesConstant;
import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.constants.ArchivePolicyKeyConstant;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveObject;
import openbackup.data.protection.access.provider.sdk.archive.v2.ArchiveCopyMetadata;
import openbackup.data.protection.access.provider.sdk.archive.v2.ArchiveTask;
import openbackup.data.protection.access.provider.sdk.backup.PredefineBackupParameters;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryRoleEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JSONObjectCovert;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.service.DeployTypeService;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.MapUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * 统一框架归档任务管理器
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/12
 **/
@Slf4j
@Component
public class ArchiveTaskManager {
    private final ArchiveTaskService archiveTaskService;

    private final ArchiveUnifiedRestApi archiveUnifiedRestApi;

    private final ProviderManager providerManager;

    private final CopyRestApi copyRestApi;

    @Autowired
    private ClusterQueryService clusterQueryService;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private TaskRepositoryManager taskRepositoryManager;


    /**
     * 归档任务管理构造函数
     *
     * @param archiveTaskService    归档任务服务
     * @param archiveUnifiedRestApi 归档统一rest api
     * @param providerManager       服务提供者
     * @param copyRestApi           副本请求
     */
    public ArchiveTaskManager(ArchiveTaskService archiveTaskService, ArchiveUnifiedRestApi archiveUnifiedRestApi,
        ProviderManager providerManager, CopyRestApi copyRestApi) {
        this.archiveTaskService = archiveTaskService;
        this.archiveUnifiedRestApi = archiveUnifiedRestApi;
        this.providerManager = providerManager;
        this.copyRestApi = copyRestApi;
    }

    /**
     * 开始归档任务，任务下发到dme
     *
     * @param archiveObject 归档任务信息
     */
    public void start(ArchiveObject archiveObject) {
        final String jobId = archiveObject.getJobId();
        final String requestId = archiveObject.getRequestId();
        final String localEsn = clusterQueryService.getCurrentClusterEsn();
        log.info("Archive task start, requestId:{}, jobId:{}, localEsn:{}.", requestId, jobId, localEsn);
        ArchiveTask archiveTask = new ArchiveTask();
        try {
            archiveTask.setTaskId(jobId);
            archiveTask.setRequestId(requestId);
            archiveTask.setOriginCopyId(archiveObject.getCopyId());
            archiveTask.setArchiveCopyId(this.archiveTaskService.genArchiveCopyIdAndSetToContext(requestId));
            archiveTask.setChainId(this.getChainId(archiveObject.getCopyId()));
            archiveTask.setLocalEsn(localEsn);
            // 根据归档策略填充归档任务参数
            this.fillParamsFromArchivePolicy(archiveTask, archiveObject.getPolicy());
            // 填充归档的聚合参数
            this.fillParamsFromCopy(archiveTask, archiveObject.getCopyId());
            this.archiveUnifiedRestApi.createArchiveTask(archiveTask);
            log.info("Archive task send to dme_archive success, requestId:{}, taskId:{}, archiveCopyId:{}", requestId,
                    archiveTask.getTaskId(), archiveTask.getArchiveCopyId());
            // 修改任务状态为运行中
            this.archiveTaskService.updateJobStatusRunning(jobId);
        } finally {
            // 清理 多存储密码
            cleanAuthPwd(archiveTask);
        }
    }

    private void fillParamsFromCopy(ArchiveTask archiveTask, String copyId) {
        Copy copy = copyRestApi.queryCopyByID(copyId, true);
        final JSONObject resourcePropertiesJsonObject = JSONObjectCovert.covertLowerUnderscoreKeyToLowerCamel(
                JSONObject.fromObject(copy.getResourceProperties()));
        final ProtectedObject protectedObject = resourcePropertiesJsonObject.toBean(ProtectedObject.class);
        Map<String, Object> extParameters = protectedObject.getExtParameters();
        if (extParameters == null) {
            return;
        }
        Boolean archiveResAutoIndex =
            MapUtils.getBoolean(extParameters, CopyResourcePropertiesConstant.ARCHIVE_RES_AUTO_INDEX, null);
        if (!VerifyUtil.isEmpty(archiveResAutoIndex)) {
            archiveTask.addAdvanceParam(ArchiveTask.ENABLE_AUTO_INDEX, archiveResAutoIndex);
        }
        archiveTask.addAdvanceParam(ArchiveTask.ENABLE_SMALL_FILE_AGGREGATION,
                MapUtils.getBoolean(extParameters,
                        CopyResourcePropertiesConstant.SMALL_FILE_AGGREGATION, Boolean.FALSE));
        archiveTask.addAdvanceParam(ArchiveTask.AGGREGATION_FILE_SIZE,
                MapUtils.getIntValue(extParameters,
                        CopyResourcePropertiesConstant.AGGREGATION_FILE_SIZE,
                        CopyResourcePropertiesConstant.DEFAULT_AGGREGATION_FILE_SIZE));
        archiveTask.addAdvanceParam(ArchiveTask.AGGREGATION_FILE_MAX_SIZE,
                MapUtils.getIntValue(extParameters,
                        CopyResourcePropertiesConstant.AGGREGATION_FILE_MAX_SIZE,
                        CopyResourcePropertiesConstant.DEFAULT_AGGREGATION_FILE_MAX_SIZE));
    }

    private void fillParamsFromArchivePolicy(ArchiveTask archiveTask, String policyJsonString) {
        // 查询并设置归档存储信息
        final JSONObject extParameters = this.getPolicyExtParameters(policyJsonString);
        final StorageRepository archiveRepository = this.archiveTaskService.getRepositoryFromPolicyExtParameters(
            extParameters, true);
        List<StorageRepository> repositories = new ArrayList<>();
        repositories.add(archiveRepository);
        // 添加本地存储
        if (!deployTypeService.isE1000()) {
            repositories.add(this.archiveTaskService.buildLocalStorageRepository());
        }
        // 设置多存储信息,目前只支持dws在备份时使用的备份存储库
        Copy copy = copyRestApi.queryCopyByID(archiveTask.getOriginCopyId(), true);
        log.info("Archive task request:{} backup storage id:{}.", archiveTask.getRequestId(), copy.getStorageId());
        // 填充存储库信息
        repositories
                .addAll(taskRepositoryManager.getStorageRepositories(copy.getProperties(), copy.getStorageUnitId()));
        if (deployTypeService.isE1000()) {
            repositories.forEach(repository -> repository.setRole(RepositoryRoleEnum.MASTER_ROLE.getRoleType()));
        }
        archiveTask.setRepositories(repositories);
        // 查询并设置qos信息
        final String qosId = extParameters.getString(PredefineBackupParameters.QOS_ID.getKey());
        this.archiveTaskService.queryQos(qosId).ifPresent(archiveTask::setQos);
        // 设置高级参数
        archiveTask.addAdvanceParam(ArchiveTask.ENABLE_AUTO_INDEX,
                MapUtils.getBoolean(extParameters, ArchivePolicyKeyConstant.AUTO_INDEX_KEY, Boolean.FALSE));
        archiveTask.addAdvanceParam(ArchiveTask.DRIVER_COUNT,
                MapUtils.getIntValue(extParameters, ArchiveTask.DRIVER_COUNT, 1));
        archiveTask.addAdvanceParam(ArchiveTask.ENABLE_SPEED_UP,
                MapUtils.getBoolean(extParameters, ArchivePolicyKeyConstant.NET_SPEED_UP_KEY, Boolean.FALSE));
    }

    private String getEsnFromCopy(Copy copy) {
        return JSONObject.fromObject(copy).getJSONObject("properties").getJSONArray("repositories")
            .getJSONObject(0).getJSONObject("extendInfo").getString("esn");
    }

    private JSONObject getPolicyExtParameters(String policyJsonString) {
        final JSONObject policy = JSONObject.fromObject(policyJsonString);
        // 获取SLA归档策略的高级参数
        return policy.getJSONObject(ArchivePolicyKeyConstant.EXT_PARAMETERS_KEY);
    }

    private String getChainId(String copyId) {
        final Copy archiveCopy = this.archiveTaskService.getArchiveCopy(copyId);
        return archiveCopy.getChainId();
    }

    /**
     * 归档任务成功
     *
     * @param requestId 任务请求id
     * @param jobStatus 任务状态
     * @param extendsInfo 任务扩展信息
     */
    public void archiveSuccess(String requestId, DmcJobStatus jobStatus, Map extendsInfo) {
        final CopyInfo copyInfo = this.archiveTaskService.buildAndSaveArchiveCopy(requestId, extendsInfo);
        this.uploadCopyToArchiveStorage(requestId, copyInfo);
        this.archiveTaskService.sendArchiveDoneMessage(requestId, jobStatus);
    }

    private void uploadCopyToArchiveStorage(String requestId, CopyInfo copyInfo) {
        ArchiveCopyMetadata copyMetadata = new ArchiveCopyMetadata();
        copyMetadata.setTaskId(requestId);
        copyMetadata.setPmMetadata(JSONObject.fromObject(copyInfo).toString());
        archiveUnifiedRestApi.updateCopyMetadata(copyMetadata);
    }

    /**
     * 归档任务失败
     *
     * @param requestId 任务请求id
     * @param jobStatus 任务状态
     */
    public void archiveFailed(String requestId, DmcJobStatus jobStatus) {
        this.archiveTaskService.sendArchiveDoneMessage(requestId, jobStatus);
    }

    private void cleanAuthPwd(ArchiveTask archiveTask) {
        archiveTask.getRepositories().forEach(StorageRepository::cleanAuth);
    }
}
