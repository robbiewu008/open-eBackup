package openbackup.system.base.sdk.job.model;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.request.JobSchedulePolicy;

import lombok.Data;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Job Guard Info
 *
 * @author l00272247
 * @since 2021-03-10
 */
@Data
public class JobExamineInfo {
    private JobSchedulePolicy policy;
    private JobGuardStatus status;
    private List<String> cause;
    private JSONObject params;

    /**
     * get status
     *
     * @param items items
     * @return status
     */
    public static JobGuardStatus getStatus(List<JobExamineInfo> items) {
        if (VerifyUtil.isEmpty(items)) {
            return null;
        }
        List<JobGuardStatus> statusList = items.stream().map(JobExamineInfo::getStatus).collect(Collectors.toList());
        for (JobGuardStatus status : Arrays.asList(JobGuardStatus.CANCELLED, JobGuardStatus.PENDING)) {
            if (statusList.contains(status)) {
                return status;
            }
        }
        return null;
    }
}
