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
package openbackup.datamover.core.listener;

import openbackup.data.access.framework.core.dao.ProtectionPlanEntityDao;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.listener.v1.job.JobListener;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.schedule.ScheduleRestApi;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.kafka.support.Acknowledgment;

import java.util.ArrayList;
import java.util.UUID;

/**
 * JobListener LLT
 *
 */

@RunWith(PowerMockRunner.class)
@PrepareForTest(JobListener.class)
@AutoConfigureMockMvc
public class JobListenerTest {
    @Mock
    private ProviderManager providerRegistry;

    @Mock
    private ScheduleRestApi scheduleRestApi;

    @Mock
    private KafkaTemplate kafkaTemplate;

    @InjectMocks
    private JobListener jobListener;

    @Mock
    private ProtectionPlanEntityDao protectionPlanEntityDao;

    @Mock
    private JobCenterRestApi jobCenterRestApi;

    @Test
    public void testStopJob() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        JobBo jobBo1 = new JobBo();
        JobBo jobBo2 = new JobBo();
        jobBo1.setAssociativeId(UUID.randomUUID().toString());
        jobBo2.setAssociativeId(UUID.randomUUID().toString());
        jobBo1.setType(JobTypeEnum.BACKUP.getValue());
        jobBo2.setType(JobTypeEnum.RESTORE.getValue());
        jobBo1.setStatus(JobStatusEnum.RUNNING.name());
        jobBo2.setStatus(JobStatusEnum.RUNNING.name());
        jobBo1.setSourceSubType(ResourceSubTypeEnum.ORACLE.getType());
        jobBo2.setSourceSubType(ResourceSubTypeEnum.VMWARE.getType());
        ArrayList<JobBo> jobBos = new ArrayList<>();
        jobBos.add(jobBo1);
        jobBos.add(jobBo2);
        JSONArray jsonArray = JSONArray.fromObject(jobBos);
        JobProvider jobProvider = PowerMockito.mock(JobProvider.class);
        PowerMockito.when(providerRegistry.findProvider(ArgumentMatchers.any(), ArgumentMatchers.any()))
                .thenReturn(jobProvider);

        jobListener.stopJob(jsonArray.toString(), acknowledgment);
        Mockito.verify(providerRegistry, Mockito.times(2)).findProvider(ArgumentMatchers.any(), ArgumentMatchers.any());
    }


}
