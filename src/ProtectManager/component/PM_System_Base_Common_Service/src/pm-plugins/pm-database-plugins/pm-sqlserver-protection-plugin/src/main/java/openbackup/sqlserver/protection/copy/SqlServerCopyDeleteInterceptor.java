package openbackup.sqlserver.protection.copy;

import static openbackup.data.access.framework.copy.mng.util.CopyUtil.getCopiesBetweenTwoCopy;

import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupChangeCauseEnum;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupModifyReq;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * SQL Server副本删除拦截器
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-04
 */
@Slf4j
@Component
public class SqlServerCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    private final SqlServerBaseService sqlServerBaseService;

    private final ResourceService resourceService;

    /**
     * Constructor
     *
     * @param copyRestApi 副本接口
     * @param sqlServerBaseService SqlServer基础服务
     * @param resourceService resourceService
     * @param resourceService protectObjectService
     */
    public SqlServerCopyDeleteInterceptor(CopyRestApi copyRestApi, SqlServerBaseService sqlServerBaseService,
        ResourceService resourceService) {
        super(copyRestApi, resourceService);
        this.sqlServerBaseService = sqlServerBaseService;
        this.resourceService = resourceService;
    }

    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType(),
            ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType(), ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType(),
            ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType()).contains(subType);
    }

    @Override
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        task.setIsForceDeleted(true);
        if (!super.isResourceExists(task)) {
            return;
        }
        TaskEnvironment protectEnv = task.getProtectEnv();
        Map<String, String> envExtendInfo = Optional.of(protectEnv.getExtendInfo()).orElse(new HashMap<>());
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        protectEnv.setExtendInfo(envExtendInfo);
        task.setProtectEnv(protectEnv);
        List<TaskEnvironment> nodes = sqlServerBaseService.queryNodeList(task.getProtectObject().getUuid());
        task.getProtectEnv().setNodes(nodes);
        // sqlserver 备份副本无需传入Repositories信息，否则会报挂载失败错误
        // 归档副本需要传入（框架会自动传入），否则会卡主
        if (!copy.getIsArchived()) {
            task.setRepositories(Collections.emptyList());
        }
    }

    @Override
    public void finalize(Copy copy, TaskCompleteMessageBo taskMessage) {
        int jobStatus = taskMessage.getJobStatus();
        String requestId = taskMessage.getJobRequestId();
        log.info("[SQL Server]copy delete post process. job status is {}, request id: {}", jobStatus, requestId);
        if (!Objects.equals(DmeJobStatusEnum.fromStatus(jobStatus), DmeJobStatusEnum.SUCCESS)) {
            return;
        }
        List<String> relatedResource = addResourcesToSetNextFull(copy, requestId);
        if (VerifyUtil.isEmpty(relatedResource)) {
            log.info("[SQL Server]related resource is empty, request id: {}, copy id: {}", requestId, copy.getUuid());
            return;
        }
        log.info("[SQL Server]related resource is: {}, request id: {}, copy id: {}", copy.getResourceId(), requestId,
            copy.getUuid());
        // 副本删除任务成功之后，也需要在后置处理中，指定对应资源的下次备份任务为全量
        NextBackupModifyReq nextBackupModifyReq = NextBackupModifyReq.build(relatedResource,
            NextBackupChangeCauseEnum.NEWEST_FULL_OR_DIFFERENCE_COPY_DELETE_SUCCESS_TO_FULL);
        resourceService.modifyNextBackup(nextBackupModifyReq, false);
    }

    @Override
    protected List<String> getCopiesCopyTypeIsFull(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        if (isContainsMorePreviousFullCopy(thisCopy)) {
            return getCopiesBetweenTwoCopy(copies, thisCopy, nextFullCopy).stream()
                .filter(copy -> copy.getBackupType() != BackupTypeConstants.LOG.getAbBackupType())
                .map(Copy::getUuid)
                .collect(Collectors.toList());
        }
        return getCopiesBetweenTwoCopy(copies, thisCopy, nextFullCopy).stream()
            .map(Copy::getUuid)
            .collect(Collectors.toList());
    }

    private boolean isContainsMorePreviousFullCopy(Copy thisCopy) {
        List<Copy> copies = copyRestApi.queryCopiesByResourceId(thisCopy.getResourceId());
        return copies.stream()
            .anyMatch(copy -> copy.getBackupType() == BackupTypeConstants.FULL.getAbBackupType()
                && copy.getGn() < thisCopy.getGn());
    }

    @Override
    protected List<String> getCopiesCopyTypeIsLog(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        List<Copy> logCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.LOG);
        return CopyUtil.getCopyUuidsBetweenTwoCopy(logCopies, thisCopy, nextFullCopy);
    }

    /**
     * 增量副本时逻辑处理
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsCumulativeIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        return Collections.emptyList();
    }

    @Override
    protected void supplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        task.setAgents(sqlServerBaseService.convertNodeListToAgents(copy.getResourceId()));
    }
}
