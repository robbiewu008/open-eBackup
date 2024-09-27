package openbackup.data.protection.access.provider.sdk.copy;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.enums.AgentMountTypeEnum;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.sdk.copy.model.Copy;

import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * 副本删除拦截器
 *
 * @author h30027154
 * @version OceanProtect X8000 1.2.1
 * @since 2022-06-15
 */
public interface CopyDeleteInterceptor extends DataProtectionProvider<String> {
    /**
     * 副本删除拦截方法
     *
     * @param task DeleteCopyTask
     * @param copy CopyInfoBo
     */
    void initialize(DeleteCopyTask task, CopyInfoBo copy);

    /**
     * 副本删除后置处理方法
     *
     * @param copy 副本
     * @param taskMessage 任务信息
     */
    default void finalize(Copy copy, TaskCompleteMessageBo taskMessage) {
    }

    /**
     * 收集与该副本相关联的需要删除的副本（不返回此副本本身），按删除顺序返回
     *
     * @param copyId 副本ID
     * @return 需要删除的关联的副本
     */
    default List<String> getAssociatedCopy(String copyId) {
        return Collections.emptyList();
    }

    /**
     * 备份所使用的挂载类型
     *
     * @param backupTask 任务对象
     * @return agent挂载类型
     */
    default Optional<AgentMountTypeEnum> getMountType(DeleteCopyTask backupTask) {
        return Optional.empty();
    }
}
