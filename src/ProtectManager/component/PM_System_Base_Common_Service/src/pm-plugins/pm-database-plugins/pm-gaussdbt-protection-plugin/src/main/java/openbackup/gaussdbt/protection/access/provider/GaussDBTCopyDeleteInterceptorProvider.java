package openbackup.gaussdbt.protection.access.provider;

import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.MountTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.gaussdbt.protection.access.provider.util.GaussDBTClusterUtil;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * GaussDBT副本删除Provider
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-01
 */
@Slf4j
@Component
public class GaussDBTCopyDeleteInterceptorProvider extends AbstractDbCopyDeleteInterceptor {
    private final ProtectedEnvironmentService environmentService;

    /**
     * 构造器注入
     *
     * @param environmentService 环境服务
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     */
    public GaussDBTCopyDeleteInterceptorProvider(ProtectedEnvironmentService environmentService,
        CopyRestApi copyRestApi, ResourceService resourceService) {
        super(copyRestApi, resourceService);
        this.environmentService = environmentService;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.GAUSSDBT.getType().equals(subType);
    }

    @Override
    protected List<String> getCopiesCopyTypeIsCumulativeIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        // 差异副本处理(删除差异到下个全量之间的所有日志和差异副本）
        List<Copy> cumulativeCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.CUMULATIVE_INCREMENT);
        List<Copy> logCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.LOG);
        cumulativeCopies.addAll(logCopies);
        return CopyUtil.getCopyUuidsBetweenTwoCopy(cumulativeCopies, thisCopy, nextFullCopy);
    }

    @Override
    protected List<String> getCopiesCopyTypeIsDifferenceIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        // 增量副本处理（删除增量到下个增量或全量之间的所有日志副本）
        Copy nextDifferenceCopy = CopyUtil.getNextDifferenceCopy(copies, thisCopy.getGn());
        Copy nextCopy = CopyUtil.getSmallerCopy(nextFullCopy, nextDifferenceCopy);
        List<Copy> logCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.LOG);
        return CopyUtil.getCopyUuidsBetweenTwoCopy(logCopies, thisCopy, nextCopy);
    }

    @Override
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        log.info("GaussDBT handle delete copy task. requestId: {}", task.getRequestId());
        // 设置高级参数挂载类型为：非全路径挂载
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
        advanceParams.put(GaussDBTConstant.MOUNT_TYPE_KEY, MountTypeEnum.FULL_PATH_MOUNT.getMountType());
        task.setAdvanceParams(advanceParams);
        ProtectedEnvironment environment = null;
        try {
            environment = environmentService.getEnvironmentById(copy.getResourceId());
        } catch (LegoCheckedException e) {
            log.error("GaussDBT copy's resource is not exist.resourceId: {}", copy.getResourceId());
            return;
        }
        TaskEnvironment taskEnvironment = BeanTools.copy(environment, TaskEnvironment::new);
        List<TaskEnvironment> nodes = GaussDBTClusterUtil.getNodesFromEnv(environment);
        taskEnvironment.setNodes(nodes);
        task.setProtectEnv(taskEnvironment);
    }
}
