package openbackup.data.protection.access.provider.sdk.job;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.system.base.common.model.job.JobBo;

/**
 * 任务强制中止回调扩展接口
 * 不同任务流程实现此接口，用于任务强制中止后恢复任务执行对象的状态
 * （比如将恢复任务将副本重置为正常）
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-12
 */
public interface JobCallbackProvider extends DataProtectionProvider<String> {
    /**
     * 不同任务类型处理任务强制中止后的状态恢复
     *
     * @param job 任务信息
     */
    void doCallback(JobBo job);
}
