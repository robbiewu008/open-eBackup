package openbackup.dameng.protection.access.interceptor;

import openbackup.dameng.protection.access.service.DamengService;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * dameng副本删除provider
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-16
 */
@Slf4j
@Component
public class DamengCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    private final DamengService damengService;

    /**
     * Constructor
     *
     * @param damengService dameng服务
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     */
    public DamengCopyDeleteInterceptor(DamengService damengService, CopyRestApi copyRestApi,
        ResourceService resourceService) {
        super(copyRestApi, resourceService);
        this.damengService = damengService;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType().equals(subType)
            || ResourceSubTypeEnum.DAMENG_CLUSTER.getType().equals(subType);
    }

    @Override
    protected void supplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        if (ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType().equals(copy.getResourceSubType())) {
            super.supplyAgent(task, copy);
            return;
        }
        task.setAgents(damengService.getEndpointList(copy.getResourceId()));
    }

    @Override
    protected List<String> getCopiesCopyTypeIsDifferenceIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        // 增量副本（返回增量副本到下个全量副本之间的增量副本）
        List<Copy> cumulativeCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.DIFFERENCE_INCREMENT);
        return CopyUtil.getCopyUuidsBetweenTwoCopy(cumulativeCopies, thisCopy, nextFullCopy);
    }

    @Override
    protected List<String> getCopiesCopyTypeIsLog(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        // 日志副本（返回本副本到下个全量副本之间的日志副本）
        List<Copy> logCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.LOG);
        return CopyUtil.getCopyUuidsBetweenTwoCopy(logCopies, thisCopy, nextFullCopy);
    }

    @Override
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        if (!super.isResourceExists(task) || super.isEnvironmentOffline(task)) {
            return;
        }
        TaskEnvironment protectEnv = task.getProtectEnv();
        Map<String, String> envExtendInfo = Optional.ofNullable(protectEnv.getExtendInfo()).orElse(new HashMap<>());
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        String subType = task.getProtectObject().getSubType();
        if (ResourceSubTypeEnum.DAMENG_CLUSTER.getType().equals(subType)) {
            envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.AP.getType());
        }
        protectEnv.setExtendInfo(envExtendInfo);
        task.setProtectEnv(protectEnv);
        List<TaskEnvironment> nodesList;
        if (ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType().equals(copy.getResourceSubType())) {
            nodesList = damengService.buildTaskHosts(task.getAgents());
        } else {
            nodesList = damengService.buildTaskNodes(task.getProtectObject().getUuid());
        }
        task.getProtectEnv().setNodes(nodesList);
    }
}
