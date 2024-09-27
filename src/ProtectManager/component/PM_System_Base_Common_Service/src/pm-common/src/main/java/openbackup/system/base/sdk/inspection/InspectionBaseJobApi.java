package openbackup.system.base.sdk.inspection;

import openbackup.system.base.common.model.job.JobSummary;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;

/**
 * 功能描述
 *
 * @author z30006621
 * @since 2021-02-04
 */
@FeignClient(name = "InspectionBaseJobApi", url = "${pm-system-base.url}/v1",
        configuration = CommonFeignConfiguration.class)
public interface InspectionBaseJobApi {
    /**
     * Get jobs summary
     *
     * @param token 认证token
     * @return JobSummary obj
     */
    @ExterAttack
    @GetMapping(value = "/jobs/summary", headers = {"x-auth-token={token}"})
    JobSummary queryJobSummary(@RequestParam("token") String token);
}
