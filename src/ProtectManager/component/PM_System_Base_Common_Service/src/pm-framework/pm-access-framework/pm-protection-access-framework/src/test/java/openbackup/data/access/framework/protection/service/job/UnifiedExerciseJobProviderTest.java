package openbackup.data.access.framework.protection.service.job;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.protection.service.job.UnifiedExerciseJobProvider;
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
 * UnifiedExerciseJobProviderTest
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-05-08
 */
@RunWith(PowerMockRunner.class)
public class UnifiedExerciseJobProviderTest {
    private UnifiedExerciseJobProvider unifiedExerciseJobProvider;

    @Mock
    private ExerciseQueryService exerciseQueryService;

    @Mock
    private JobProvider unifiedJobProvider;

    @Before
    public void init() {
        unifiedExerciseJobProvider = new UnifiedExerciseJobProvider();
        Whitebox.setInternalState(unifiedExerciseJobProvider, "exerciseQueryService", exerciseQueryService);
        Whitebox.setInternalState(unifiedExerciseJobProvider, "unifiedJobProvider", unifiedJobProvider);
    }

    @Test
    public void test_stop_job_success() {
        unifiedExerciseJobProvider.stopJob("123456");
        Mockito.verify(unifiedJobProvider).stopJob(anyString());
    }

    @Test
    public void test_fill_job_info_success() {
        Job insertJob = new Job();
        insertJob.setExerciseId("123456");
        insertJob.setExtendStr("{}");
        PowerMockito.when(exerciseQueryService.queryExercise(anyString())).thenReturn(new ExerciseBase());
        unifiedExerciseJobProvider.fillJobInfo(insertJob);
        Assert.assertTrue(insertJob.getExtendStr().contains(JobExtendInfoKeys.TRIGGER_POLICY));
    }


    @Test
    public void test_applicable_success() {
        Assert.assertTrue(unifiedExerciseJobProvider.applicable(JobTypeEnum.EXERCISE.getValue()));
    }
}