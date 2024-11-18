/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
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