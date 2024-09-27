package openbackup.system.base.common.scurity;

/**
 * 创建任务执行结果
 *
 * @author j00502557
 * @version [OceanStor ReplicationDirector V200R001C00, 2019年8月27日]
 * @since 2019-10-30
 */
public class BaseTaskResult {
    private boolean isSuccess;

    public boolean isSuccess() {
        return isSuccess;
    }

    public void setSuccess(boolean success) {
        isSuccess = success;
    }
}
