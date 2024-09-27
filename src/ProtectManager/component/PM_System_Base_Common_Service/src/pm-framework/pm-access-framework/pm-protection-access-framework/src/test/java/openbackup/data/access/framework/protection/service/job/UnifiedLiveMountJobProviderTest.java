package openbackup.data.access.framework.protection.service.job;

import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.livemount.controller.policy.response.LiveMountPolicyVo;
import openbackup.data.access.framework.livemount.service.PolicyService;
import openbackup.data.access.framework.protection.service.job.UnifiedLiveMountJobProvider;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import com.huawei.oceanprotect.exercise.entity.ExerciseBase;
import com.huawei.oceanprotect.exercise.service.ExerciseQueryService;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

/**
 * UnifiedLiveMountJobProviderTest
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-05-08
 */
@RunWith(PowerMockRunner.class)
public class UnifiedLiveMountJobProviderTest {
    private UnifiedLiveMountJobProvider unifiedLiveMountJobProvider;

    @Mock
    private PolicyService liveMountPolicyService;

    @Mock
    private ExerciseQueryService exerciseQueryService;

    @Mock
    private JobProvider unifiedJobProvider;

    @Before
    public void init() {
        unifiedLiveMountJobProvider = new UnifiedLiveMountJobProvider();
        Whitebox.setInternalState(unifiedLiveMountJobProvider, "exerciseQueryService", exerciseQueryService);
        Whitebox.setInternalState(unifiedLiveMountJobProvider, "liveMountPolicyService", liveMountPolicyService);
        Whitebox.setInternalState(unifiedLiveMountJobProvider, "unifiedJobProvider", unifiedJobProvider);
    }

    @Test
    public void test_stop_job_success() {
        unifiedLiveMountJobProvider.stopJob("123456");
        Mockito.verify(unifiedJobProvider).stopJob(anyString());
    }

    @Test
    public void test_fill_job_info_success() {
        Job insertJob = new Job();
        insertJob.setExtendStr("{\"liveMountPolicyId\":\"123\"}");
        PowerMockito.when(liveMountPolicyService.getPolicy(anyString())).thenReturn(new LiveMountPolicyVo());
        unifiedLiveMountJobProvider.fillJobInfo(insertJob);
        Assert.assertTrue(insertJob.getExtendStr().contains(JobExtendInfoKeys.TRIGGER_POLICY));
        insertJob.setExerciseId("123456");
        insertJob.setExtendStr("{}");
        PowerMockito.when(exerciseQueryService.queryExercise(anyString())).thenReturn(new ExerciseBase());
        unifiedLiveMountJobProvider.fillJobInfo(insertJob);
        Assert.assertTrue(insertJob.getExtendStr().contains(JobExtendInfoKeys.TRIGGER_POLICY));
    }

    @Test
    public void test_applicable_success() {
        Assert.assertTrue(unifiedLiveMountJobProvider.applicable(JobTypeEnum.LIVE_MOUNT.getValue()));
    }
}