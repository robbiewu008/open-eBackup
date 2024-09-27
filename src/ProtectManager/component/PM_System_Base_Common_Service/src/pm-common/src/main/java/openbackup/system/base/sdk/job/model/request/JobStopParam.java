package openbackup.system.base.sdk.job.model.request;

import lombok.Data;

/**
 * 任务停止参数
 *
 * @author g00500588
 * @since 2021/10/14
 */
@Data
public class JobStopParam {
    private boolean backupEngineCancelable;
    private boolean enforceStop;
}
