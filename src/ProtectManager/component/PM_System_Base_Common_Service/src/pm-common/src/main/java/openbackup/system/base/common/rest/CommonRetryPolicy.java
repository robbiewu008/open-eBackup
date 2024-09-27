package openbackup.system.base.common.rest;

import openbackup.system.base.common.constants.FeignClientConstant;

import lombok.Data;

/**
 * Common Retry Policy
 *
 * @author l00272247
 * @since 2021-05-31
 */
@Data
public class CommonRetryPolicy {
    /**
     * FeignClient Retry重试次数
     */
    private int attempts = 6;

    /**
     * FeignClient Retry 间隔周期(ms)
     */
    private long waitTime = FeignClientConstant.PERIOD;
}
