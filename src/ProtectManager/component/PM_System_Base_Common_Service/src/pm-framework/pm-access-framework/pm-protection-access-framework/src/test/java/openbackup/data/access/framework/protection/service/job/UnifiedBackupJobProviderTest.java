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

import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.access.framework.protection.service.job.UnifiedBackupJobProvider;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import com.baomidou.mybatisplus.core.MybatisConfiguration;
import com.baomidou.mybatisplus.core.metadata.TableInfoHelper;

import org.apache.ibatis.builder.MapperBuilderAssistant;
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
 * UnifiedBackupJobProviderTest
 *
 */
@RunWith(PowerMockRunner.class)
public class UnifiedBackupJobProviderTest {
    private UnifiedBackupJobProvider unifiedBackupJobProvider;

    @Mock
    private SlaQueryService slaQueryService;

    @Mock
    private FunctionSwitchService functionSwitchService;

    @Mock
    private JobProvider unifiedJobProvider;

    @Mock
    private ProtectedObjectMapper protectedObjectMapper;

    @Before
    public void init() {
        TableInfoHelper.initTableInfo(new MapperBuilderAssistant(new MybatisConfiguration(), ""), ProtectedObjectPo.class);
        unifiedBackupJobProvider = new UnifiedBackupJobProvider();
        Whitebox.setInternalState(unifiedBackupJobProvider, "slaQueryService", slaQueryService);
        Whitebox.setInternalState(unifiedBackupJobProvider, "functionSwitchService", functionSwitchService);
        Whitebox.setInternalState(unifiedBackupJobProvider, "unifiedJobProvider", unifiedJobProvider);
        Whitebox.setInternalState(unifiedBackupJobProvider, "protectedObjectMapper", protectedObjectMapper);
    }

    @Test
    public void test_stop_job_success() {
        unifiedBackupJobProvider.stopJob("123456");
        Mockito.verify(unifiedJobProvider).stopJob(anyString());
    }

    @Test
    public void test_fill_job_info_success() {
        ProtectedObjectPo protectedObjectPo = new ProtectedObjectPo();
        protectedObjectPo.setExtParameters("{}");
        PowerMockito.when(protectedObjectMapper.selectOne(any())).thenReturn(protectedObjectPo);
        Job insertJob = new Job();
        insertJob.setExtendStr("{\"slaId\":\"123456\"}");
        PowerMockito.when(slaQueryService.querySlaById(anyString())).thenReturn(new SlaDto());
        unifiedBackupJobProvider.fillJobInfo(insertJob);
        Assert.assertTrue(insertJob.getExtendStr().contains(JobExtendInfoKeys.TRIGGER_POLICY));
    }

    @Test
    public void test_applicable_success() {
        Assert.assertTrue(unifiedBackupJobProvider.applicable(JobTypeEnum.BACKUP.getValue()));
    }
}
