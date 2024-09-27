package openbackup.system.base.sdk.job.api;

import openbackup.system.base.sdk.job.model.request.JobScheduleConfig;

/**
 * JobCenter 本地调用API接口定义
 *
 * @author y00559272
 * @since 2021-10-21
 */
public interface JobCenterNativeApi {
    /**
     * 更新job调度配置
     *
     * @param config 调度配置类
     */
    void updateJobSchedulePolicy(JobScheduleConfig config);
}
