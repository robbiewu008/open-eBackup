package openbackup.data.access.framework.copy.index.service;

import openbackup.data.protection.access.provider.sdk.job.Task;
import openbackup.data.protection.access.provider.sdk.restore.RestoreObject;
import openbackup.system.base.security.exterattack.ExterAttack;

/**
 * 虚拟化应用细粒度恢复服务接口
 *
 * @author y30037959
 * @since 2023-06-12
 */
public interface IvmFileLevelRestoreService {
    /**
     * 文件细粒度恢复
     *
     * @param restoreObject 恢复对象
     * @param snapMetaData 快照元数据
     * @return 任务信息
     */
    @ExterAttack
    Task fileLevelRestore(RestoreObject restoreObject, String snapMetaData);

    /**
     * 文件下载
     *
     * @param restoreObject 恢复对象
     * @param snapMetaData 快照元数据
     * @return 任务信息
     */
    @ExterAttack
    Task download(RestoreObject restoreObject, String snapMetaData);
}
