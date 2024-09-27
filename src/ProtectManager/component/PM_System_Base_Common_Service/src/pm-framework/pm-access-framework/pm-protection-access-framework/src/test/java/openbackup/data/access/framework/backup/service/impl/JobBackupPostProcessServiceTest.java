package openbackup.data.access.framework.backup.service.impl;

import static org.mockito.ArgumentMatchers.any;

import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.data.access.framework.backup.service.impl.JobBackupPostProcessService;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;

@RunWith(PowerMockRunner.class)
@PrepareForTest({JobBackupPostProcessService.class, JobService.class})
public class JobBackupPostProcessServiceTest {
    @InjectMocks
    private JobBackupPostProcessService jobBackupPostProcessService;

    @Mock
    private JobService jobService;

    @Mock
    private ProtectObjectRestApi protectObjectRestApi;

    @Test
    public void test_onBackupJobFail() {
        JobBo jobBo = new JobBo();
        jobBo.setJobId("id");
        PowerMockito.when(jobService.queryJob("id")).thenReturn(jobBo);
        PageListResponse<JobLogBo> response = new PageListResponse<>();
        response.setRecords(Collections.emptyList());
        PowerMockito.doReturn(response).when(jobService).queryJobLogs(any(), any(), any(), any());
        jobBackupPostProcessService.onBackupJobFail("id");
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setLogInfo("agent_access_remote_storage_fail_label");

        response.setRecords(Collections.singletonList(jobLogBo));
        PowerMockito.doReturn(response).when(jobService).queryJobLogs(any(), any(), any(), any());
        PowerMockito.doNothing().when(protectObjectRestApi).modifyProtectedObjectExtParam(any());
        jobBackupPostProcessService.onBackupJobFail("id");
    }

    @Test
    public void test_onBackupJobSuccess() {
        JobBo jobBo = new JobBo();
        jobBo.setSourceId("id");
        PowerMockito.when(jobService.queryJob("id")).thenReturn(jobBo);
        jobBackupPostProcessService.onBackupJobSuccess("id");
    }
}
