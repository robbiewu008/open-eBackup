package openbackup.system.base.sdk.accesspoint;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.accesspoint.model.StopPlanBo;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * JobCenter Client Service
 *
 * @author h30003246
 * @since 2020-07-16
 */
@FeignClient(name = "PlanRestApi", url = "${service.url.pm-dm-access-point}/v1/internal/jobs",
    configuration = CommonFeignConfiguration.class)
public interface JobRestApi {
    /**
     * stop plan
     *
     * @param jobId      plan id
     * @param stopPlanBo job stop bo
     */
    @PutMapping("/{jobId}/action/stop")
    void stopTask(@PathVariable("jobId") String jobId, @RequestBody StopPlanBo stopPlanBo);
}
