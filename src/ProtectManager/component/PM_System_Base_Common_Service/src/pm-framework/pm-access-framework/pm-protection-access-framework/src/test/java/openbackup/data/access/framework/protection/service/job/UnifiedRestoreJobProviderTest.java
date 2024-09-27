/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.access.framework.protection.service.job;

import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.protection.service.job.UnifiedRestoreJobProvider;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import com.huawei.oceanprotect.exercise.entity.ExerciseBase;
import com.huawei.oceanprotect.exercise.service.ExerciseQueryService;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
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
 * UnifiedRestoreProviderTest
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-05-08
 */
@RunWith(PowerMockRunner.class)
public class UnifiedRestoreJobProviderTest {
    private UnifiedRestoreJobProvider unifiedRestoreJobProvider;

    @Mock
    private JobProvider unifiedJobProvider;

    @Mock
    private FunctionSwitchService functionSwitchService;

    @Mock
    private ExerciseQueryService exerciseQueryService;

    @Before
    public void init() {
        unifiedRestoreJobProvider = new UnifiedRestoreJobProvider();
        Whitebox.setInternalState(unifiedRestoreJobProvider, "exerciseQueryService", exerciseQueryService);
        Whitebox.setInternalState(unifiedRestoreJobProvider, "functionSwitchService", functionSwitchService);
        Whitebox.setInternalState(unifiedRestoreJobProvider, "unifiedJobProvider", unifiedJobProvider);
    }

    @Test
    public void test_stop_job_success() {
        unifiedRestoreJobProvider.stopJob("123456");
        Mockito.verify(unifiedJobProvider).stopJob(anyString());
    }

    @Test
    public void test_fill_job_info_success() {
        Job insertJob = new Job();
        insertJob.setExerciseId("123456");
        insertJob.setExtendStr("{}");
        PowerMockito.when(exerciseQueryService.queryExercise(anyString())).thenReturn(new ExerciseBase());
        unifiedRestoreJobProvider.fillJobInfo(insertJob);
        Assert.assertTrue(insertJob.getExtendStr().contains(JobExtendInfoKeys.TRIGGER_POLICY));
    }

    @Test
    public void test_applicable_success() {
        Assert.assertTrue(unifiedRestoreJobProvider.applicable(JobTypeEnum.RESTORE.getValue()));
    }
}