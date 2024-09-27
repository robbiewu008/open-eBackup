package openbackup.access.framework.resource.schedule;

import openbackup.access.framework.resource.service.ResourceScanService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;

import lombok.extern.slf4j.Slf4j;

import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Optional;

/**
 * 资源扫描Schedule
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-11-02
 */
@Slf4j
@Component
public class ResourceScanSchedule {
    /**
     * 最大更新的扫描任务的时间 2h
     */
    private static final long MAX_UPDATE_SCAN_JOB_TIME = 2 * 60 * 60 * 1000L;

    private final ResourceScanService resourceScanService;

    private final JobService jobService;

    public ResourceScanSchedule(ResourceScanService resourceScanService, JobService jobService) {
        this.resourceScanService = resourceScanService;
        this.jobService = jobService;
    }

    /**
     * 定时刷新扫描任务状态，防止30分钟超时
     * <p>
     * 刷新时间为任务开始时间的MAX_UPDATE_SCAN_JOB_TIME小时内，刷新隔间15分钟
     */
    @Scheduled(cron = "${system.schedule.resource.refreshScanJob}")
    public void refreshManualScanJob() {
        int page = 0;
        int size = 1000;
        List<JobBo> jobBos;
        do {
            jobBos = resourceScanService.queryManualScanRunningPage(page, size);
            page++;
            for (JobBo jobBo : jobBos) {
                long deltaTime = System.currentTimeMillis() - jobBo.getStartTime();
                if (deltaTime > MAX_UPDATE_SCAN_JOB_TIME) {
                    continue;
                }
                UpdateJobRequest updateJobRequest = new UpdateJobRequest();
                updateJobRequest.setProgress(Optional.ofNullable(jobBo.getProgress()).orElse(0));
                jobService.updateJob(jobBo.getJobId(), updateJobRequest);
            }
        } while (jobBos.size() == size);
    }
}
