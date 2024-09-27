package openbackup.data.access.framework.protection.listener.v1.model;

import lombok.Data;

/**
 * 任务监控消息
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2020-10-16
 */
@Data
public class JobMonitorBo {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 资源类型
     */
    private String resourceType;

    /**
     * 任务id
     */
    private String jobId;

    /**
     * 任务类型
     */
    private String jobType;

    /**
     * 标准备份计划id
     */
    private String planId;

    /**
     * 标准备份任务实例id
     */
    private String instanceId;
}
