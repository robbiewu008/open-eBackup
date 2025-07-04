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
package openbackup.db2.protection.access.provider.backup;

import com.huawei.oceanprotect.job.sdk.JobService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.protection.service.SanClientService;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.db2.protection.access.constant.Db2Constants;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.enums.Db2ResourceStatusEnum;
import openbackup.db2.protection.access.service.Db2Service;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.SortingParamRequest;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.model.job.request.QueryJobRequest;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.collections.MapUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * db2备份拦截器
 *
 */
@Slf4j
@Component
public class Db2BackupInterceptorProvider extends AbstractDbBackupInterceptor {
    private static final List<String> NOT_INTERSECT_BACKUP_TYPES = Arrays.asList("incrementBackup", "diffBackup");

    private final ResourceService resourceService;

    private final InstanceProtectionService instanceProtectionService;

    private final Db2Service db2Service;

    private final JobService jobService;

    private final CopyRestApi copyRestApi;

    private SanClientService sanClientService;

    private ProtectedEnvironmentService protectedEnvironmentService;

    public Db2BackupInterceptorProvider(ResourceService resourceService,
        InstanceProtectionService instanceProtectionService, Db2Service db2Service, JobService jobService,
        CopyRestApi copyRestApi) {
        this.resourceService = resourceService;
        this.instanceProtectionService = instanceProtectionService;
        this.db2Service = db2Service;
        this.jobService = jobService;
        this.copyRestApi = copyRestApi;
    }

    @Autowired
    public void setSanClientService(SanClientService sanClientService) {
        this.sanClientService = sanClientService;
    }

    @Autowired
    public void setProtectedEnvironmentService(ProtectedEnvironmentService protectedEnvironmentService) {
        this.protectedEnvironmentService = protectedEnvironmentService;
    }

    @Override
    public BackupTask initialize(BackupTask backupTask) {
        backupTask.setBackupType(getBackupType(backupTask));

        super.setCopyRestoreNeedWritable(backupTask);

        // 设置保护环境扩展参数deployType
        setProtectEnvExtendInfo(backupTask);

        // 设置副本格式
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());

        // 设置速度统计方式为UBC
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);

        // 设置存储仓
        backupTask.setRepositories(setRepositories(backupTask));

        ProtectedResource resource = getResourceById(backupTask.getProtectObject().getUuid());

        // 设置nodes
        backupTask.getProtectEnv().setNodes(getEnvNodesByInstanceResource(resource));

        // 设置agents
        backupTask.setAgents(getAgentsByInstanceResource(resource));

        // 日志备份加上检查任务类型校验
        checkIsLogBackup(backupTask);

        // 检查所有Agent的状态
        checkAgentsStatus(backupTask);

        db2Service.updateHadrDatabaseStatus(backupTask.getProtectObject(), Db2ResourceStatusEnum.BACKUPING.getStatus());

        // 设置保护对象的数据量大小
        fillProtectObjectDatabaseSize(backupTask, resource);
        return backupTask;
    }

    private void checkAgentsStatus(BackupTask backupTask) {
        Map<String, String> envExtendInfo = Optional.ofNullable(backupTask.getProtectEnv().getExtendInfo())
            .orElseGet(HashMap::new);
        String clusterType = envExtendInfo.get(DatabaseConstants.CLUSTER_TYPE);
        if (!Db2ClusterTypeEnum.DPF.getType().equals(clusterType)) {
            return;
        }
        // DPF集群需保证所有Agent在线
        boolean hasOfflineAgent = backupTask.getAgents()
            .stream()
            .anyMatch(agent -> LinkStatusEnum.OFFLINE.getStatus()
                .toString()
                .equals(protectedEnvironmentService.getEnvironmentById(agent.getId()).getLinkStatus()));
        if (hasOfflineAgent) {
            log.error("The DB2 cluster has offline agent.");
            throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, "The DB2 cluster has offline agent.");
        }
    }

    private void fillProtectObjectDatabaseSize(BackupTask backupTask, ProtectedResource resource) {
        List<Endpoint> agents = backupTask.getAgents();
        String[] notConfiguredSanClientAgents = sanClientService.getAgentsNotConfiguredSanclient(agents);
        // lan-free备份查询数据量大小
        if (VerifyUtil.isEmpty(notConfiguredSanClientAgents)) {
            String dataSize = db2Service.queryDatabaseSize(resource);
            log.debug("Db2 lan free backup this database size is: {}. task id: {}.", dataSize, backupTask.getTaskId());
            Map<String, String> extendInfo = Optional.ofNullable(backupTask.getProtectObject().getExtendInfo())
                .orElse(new HashMap<>());
            extendInfo.put(Db2Constants.DATA_SIZE_KEY, dataSize);
            backupTask.getProtectObject().setExtendInfo(extendInfo);
        }
    }

    private String getBackupType(BackupTask backupTask) {
        if (checkTablespaceIsChanged(backupTask)) {
            log.info("Db2 tablespace is changed. task id: {}, resource id: {}, backup type: {}.",
                backupTask.getTaskId(), backupTask.getProtectObject().getUuid(), backupTask.getBackupType());
            return DatabaseConstants.FULL_BACKUP_TYPE;
        }
        return backupTask.getBackupType();
    }

    private boolean checkTablespaceIsChanged(BackupTask backupTask) {
        if (!ResourceSubTypeEnum.DB2_TABLESPACE.equalsSubType(backupTask.getProtectObject().getSubType())) {
            return false;
        }
        if (DatabaseConstants.FULL_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            return false;
        }
        Copy copy = copyRestApi.queryLatestBackupCopy(backupTask.getProtectObject().getUuid(), null, null);
        if (copy == null) {
            log.info("Last copy is not exists. task id: {}, resource id: {}, backup type: {}.", backupTask.getTaskId(),
                backupTask.getProtectObject().getUuid(), backupTask.getBackupType());
            return false;
        }
        return validTablespaceIsChanged(backupTask, copy);
    }

    private boolean validTablespaceIsChanged(BackupTask backupTask, Copy copy) {
        String currentTablespace = backupTask.getProtectObject().getExtendInfo().get(DatabaseConstants.TABLESPACE_KEY);
        List<String> tablespaceList = Arrays.asList(currentTablespace.split(DatabaseConstants.SPLIT_CHAR));
        String resourceJsonString = copy.getResourceProperties();
        JSONObject resourceJson = JSONObject.fromObject(resourceJsonString);
        String beforeTable = resourceJson.getJSONObject(DatabaseConstants.EXTEND_INFO)
            .getString(DatabaseConstants.TABLESPACE_KEY);
        List<String> beforeTablespaceList = Arrays.asList(beforeTable.split(DatabaseConstants.SPLIT_CHAR));
        return !(tablespaceList.containsAll(beforeTablespaceList) && beforeTablespaceList.containsAll(tablespaceList));
    }

    private QueryJobRequest buildQueryJobRequest(String resourceId) {
        QueryJobRequest conditions = new QueryJobRequest();
        conditions.setSourceId(resourceId);
        conditions.setTypes(Collections.singletonList(DatabaseConstants.BACKUP));
        conditions.setStatusList(Arrays.asList(DatabaseConstants.SUCCESS));
        return conditions;
    }

    private SortingParamRequest buildSortingParamRequest() {
        SortingParamRequest sortRule = new SortingParamRequest();
        sortRule.setOrderBy(DatabaseConstants.END_TIME);
        sortRule.setOrderType(SortingParamRequest.DES);
        return sortRule;
    }

    private PagingParamRequest buildPagingParamRequest(int startPage) {
        PagingParamRequest pageParam = new PagingParamRequest();
        pageParam.setStartPage(startPage);
        pageParam.setPageSize(IsmNumberConstant.HUNDRED);
        return pageParam;
    }

    private List<JobBo> queryProtectionObjectJobs(BackupTask backupTask) {
        QueryJobRequest conditions = buildQueryJobRequest(backupTask.getProtectObject().getUuid());
        SortingParamRequest sortRule = buildSortingParamRequest();
        int startPage = 0;
        PagingParamRequest pageParam = buildPagingParamRequest(startPage);
        List<JobBo> resourceJobs = new ArrayList<>();
        PageListResponse<JobBo> pageListResponse;
        do {
            pageListResponse = jobService.queryJobs(conditions, pageParam, sortRule);
            resourceJobs.addAll(pageListResponse.getRecords());
            startPage++;
            pageParam = buildPagingParamRequest(startPage);
        } while (pageListResponse.getRecords().size() >= IsmNumberConstant.HUNDRED);
        return resourceJobs;
    }

    private JobBo getLatestFullBackupJob(List<JobBo> resourceJobs) {
        return resourceJobs.stream()
            .filter(jobBo -> BackupTypeConstants.FULL.getBackupType().equals(extractBackupType(jobBo)))
            .findFirst()
            .orElse(null);
    }

    private String extractBackupType(JobBo jobBo) {
        JSONObject jsonObject = JSONObject.fromObject(jobBo.getExtendStr());
        return jsonObject.getString(DatabaseConstants.BACKUP_TYPE_KEY);
    }

    private List<String> getBackupJobsAfterLatestFull(List<JobBo> resourceJobs, JobBo latestFullBackupJob) {
        return resourceJobs.stream()
            .filter(jobBo -> jobBo.getStartTime() > latestFullBackupJob.getStartTime())
            .map(jobBo -> extractBackupType(jobBo))
            .map(backupType -> BackupTypeConstants.convert2DmeBackType(backupType))
            .filter(backupType -> !DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType))
            .collect(Collectors.toList());
    }

    private ProtectedResource getResourceById(String resourceId) {
        return resourceService.getResourceById(resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "Protected resource not exist. uuid: " + resourceId));
    }

    private void setProtectEnvExtendInfo(BackupTask backupTask) {
        Map<String, String> envExtendInfo = Optional.ofNullable(backupTask.getProtectEnv().getExtendInfo())
            .orElseGet(HashMap::new);
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, Db2ClusterTypeEnum.getDeployType(backupTask.getProtectEnv()
            .getExtendInfo()
            .getOrDefault(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.SINGLE.getType())));
        backupTask.getProtectEnv().setExtendInfo(envExtendInfo);
    }

    private List<StorageRepository> setRepositories(BackupTask backupTask) {
        List<StorageRepository> repositories = backupTask.getRepositories();
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(IsmNumberConstant.ZERO),
                StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            repositories.add(logRepository);
        }
        StorageRepository metaRepository = BeanTools.copy(repositories.get(IsmNumberConstant.ZERO),
            StorageRepository::new);
        metaRepository.setType(RepositoryTypeEnum.META.getType());
        repositories.add(metaRepository);
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(IsmNumberConstant.ZERO),
            StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        return repositories;
    }

    private List<TaskEnvironment> getEnvNodesByInstanceResource(ProtectedResource resource) {
        if (ResourceSubTypeEnum.DB2_DATABASE.equalsSubType(resource.getSubType())) {
            return extractEnvNodesByInstance(resource.getParentUuid());
        }
        return extractEnvNodesByInstance(resource.getExtendInfoByKey(DatabaseConstants.INSTANCE_UUID_KEY));
    }

    private List<TaskEnvironment> extractEnvNodesByInstance(String instanceId) {
        ProtectedResource instance = getResourceById(instanceId);
        if (ResourceSubTypeEnum.DB2_INSTANCE.equalsSubType(instance.getSubType())) {
            return instanceProtectionService.extractEnvNodesBySingleInstance(instance);
        }
        return instanceProtectionService.extractEnvNodesByClusterInstance(instance);
    }

    private List<Endpoint> getAgentsByInstanceResource(ProtectedResource resource) {
        List<TaskEnvironment> nodeList = getEnvNodesByInstanceResource(resource);
        return nodeList.stream()
            .map(node -> new Endpoint(node.getUuid(), node.getEndpoint(), node.getPort()))
            .collect(Collectors.toList());
    }

    @Override
    public void finalize(PostBackupTask postBackupTask) {
        ProtectedResource protectedResource = resourceService.getResourceById(
            postBackupTask.getProtectedObject().getResourceId())
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected resource not exist."));
        db2Service.updateHadrDatabaseStatus(BeanTools.copy(protectedResource, TaskResource::new),
            Db2ResourceStatusEnum.NORMAL.getStatus());

        // 如果后置任务失败，不执行删除归档日志
        if (!ProviderJobStatusEnum.SUCCESS.equals(postBackupTask.getJobStatus())) {
            log.info("finalize backup job not success");
            return;
        }
        // 获取副本信息
        CopyInfo copyInfoStr = postBackupTask.getCopyInfo();
        String copyPropertiesStr = copyInfoStr.getProperties();
        JSONObject copyProperties = JSONObject.fromObject(copyPropertiesStr);
        String resourcePropertiesStr = copyInfoStr.getResourceProperties();
        JSONObject resourceProperties = JSONObject.fromObject(resourcePropertiesStr);
        // 判断是否开启了归档日志
        String deleteLog = copyProperties.getString(Db2Constants.DELETELOG);
        if (VerifyUtil.isEmpty(deleteLog) || Db2Constants.FALSE.equals(deleteLog)) {
            log.info("finalize delete archive log is false");
            return;
        }

        // 获取agent资源id
        String agent_id = copyProperties.getString(Db2Constants.UUID);
        // 获取数据库资源id
        String resourceId = postBackupTask.getProtectedObject().getResourceId();
        // 组装参数
        String resource_extendInfoStr = resourceProperties.getString(Db2Constants.EXTENDINFO);
        JSONObject resource_extendInfo = JSONObject.fromObject(resource_extendInfoStr);
        String clusterType = resource_extendInfo.getString(Db2Constants.CLUSTERTYPE);
        String databaseName = copyProperties.getString(Db2Constants.DATABASENAME);
        String userName = copyProperties.getString(Db2Constants.USERNAME);
        String cachePath = copyProperties.getString(Db2Constants.CACHEPATH);
        String sinceTimeStamp = copyProperties.getString(Db2Constants.SINCETIMESTAMP);
        String endTime = copyProperties.getString(Db2Constants.END_TIME);
        String beginTime = copyProperties.getString(Db2Constants.BEGIN_TIME);

        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(Db2Constants.END_TIME, endTime);
        extendInfo.put(Db2Constants.BEGIN_TIME, beginTime);
        extendInfo.put(Db2Constants.DATABASENAME, databaseName);
        extendInfo.put(Db2Constants.USERNAME, userName);
        extendInfo.put(Db2Constants.CLUSTERTYPE, clusterType);
        extendInfo.put(Db2Constants.CACHEPATH, cachePath);
        extendInfo.put(Db2Constants.SINCETIMESTAMP, sinceTimeStamp);
        db2Service.deleteLogFromProductEnv(protectedResource, extendInfo, agent_id);
        log.info("delete archivelog end");
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return Arrays.asList(ResourceSubTypeEnum.DB2_DATABASE.getType(), ResourceSubTypeEnum.DB2_TABLESPACE.getType())
            .contains(resourceSubType);
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return true;
    }

    private void checkIsLogBackup(BackupTask backupTask) {
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            Map<String, String> advanceParams = backupTask.getAdvanceParams();
            if (MapUtils.getBooleanValue(advanceParams, Db2Constants.AUTO_FULL_BACKUP, false)) {
                advanceParams.put(Db2Constants.IS_CHECK_BACKUP_JOB_TYPE, "true");
            }
        }
    }
}
