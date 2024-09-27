package openbackup.data.access.framework.protection.service.job;

import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * 统一备份框架Job提供者，负责任务的终止
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-29
 */
@Slf4j
@Component("unifiedJobProvider")
public class UnifiedJobProvider implements JobProvider {
    private final DmeUnifiedRestApi dmeUnifiedRestApi;

    /**
     * 构造函数
     *
     * @param dmeUnifiedRestApi DME统一备份框架REST接口
     */
    public UnifiedJobProvider(DmeUnifiedRestApi dmeUnifiedRestApi) {
        this.dmeUnifiedRestApi = dmeUnifiedRestApi;
    }

    @Override
    public void stopJob(String jobId) {
        log.debug("Begin to send stop job command to datamover engine. job id is {}", jobId);
        this.dmeUnifiedRestApi.abortJob(jobId, jobId);
        log.debug("Success to send stop job command to datamover engine.");
    }

    @Override
    public boolean applicable(String object) {
        return false;
    }
}
