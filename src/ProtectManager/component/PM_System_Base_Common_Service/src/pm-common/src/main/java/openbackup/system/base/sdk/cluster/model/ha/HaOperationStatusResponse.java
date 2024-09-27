package openbackup.system.base.sdk.cluster.model.ha;

import openbackup.system.base.sdk.cluster.model.JobLog;

import lombok.Data;

import java.util.List;

/**
 * HA操作结果
 *
 * @author w00607005
 * @since 2023-05-22
 */
@Data
public class HaOperationStatusResponse {
    /**
     * 任务状态
     */
    private Integer status;

    /**
     * 任务日志
     */
    private List<JobLog> jobLogs;
}
