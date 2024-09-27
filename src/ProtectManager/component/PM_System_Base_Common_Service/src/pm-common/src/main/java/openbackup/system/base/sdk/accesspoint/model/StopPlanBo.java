package openbackup.system.base.sdk.accesspoint.model;

import openbackup.system.base.sdk.job.model.JobTypeEnum;

import lombok.Data;

/**
 * Job对象
 *
 * @author w00448845
 * @version [BCManager 8.0.0]
 * @since 2020-04-15
 */
@Data
public class StopPlanBo {
    private JobTypeEnum type;

    private String sourceSubType;

    private String associativeId;

    private String requestId;

    private String jobId;
}
