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

import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.archive.ArchiveUnifiedRestApi;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;

import openbackup.data.access.framework.protection.service.job.UnifiedArchiveJobProvider;
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
 * UnifiedArchiveJobProviderTest
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-05-08
 */
@RunWith(PowerMockRunner.class)
public class UnifiedArchiveJobProviderTest {
    private UnifiedArchiveJobProvider unifiedArchiveJobProvider;

    @Mock
    private ArchiveUnifiedRestApi archiveUnifiedRestApi;

    @Mock
    private SlaQueryService slaQueryService;

    @Mock
    private FunctionSwitchService functionSwitchService;

    @Before
    public void init() {
        unifiedArchiveJobProvider = new UnifiedArchiveJobProvider(archiveUnifiedRestApi);
        Whitebox.setInternalState(unifiedArchiveJobProvider, "slaQueryService", slaQueryService);
        Whitebox.setInternalState(unifiedArchiveJobProvider, "functionSwitchService", functionSwitchService);
    }

    @Test
    public void test_stop_job_success() {
        unifiedArchiveJobProvider.stopJob("123456");
        Mockito.verify(archiveUnifiedRestApi).abortTask(anyString(), anyString());
    }

    @Test
    public void test_fill_job_info_success() {
        Job insertJob = new Job();
        insertJob.setExtendStr("{\"slaId\":\"123456\"}");
        PowerMockito.when(slaQueryService.querySlaById(anyString())).thenReturn(new SlaDto());
        unifiedArchiveJobProvider.fillJobInfo(insertJob);
        Assert.assertTrue(insertJob.getExtendStr().contains(JobExtendInfoKeys.TRIGGER_POLICY));
    }

    @Test
    public void test_applicable_success() {
        Assert.assertTrue(unifiedArchiveJobProvider.applicable(JobTypeEnum.ARCHIVE.getValue()));
    }
}