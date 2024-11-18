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
package openbackup.gaussdbt.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.sdk.job.model.request.JobSchedulePolicy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.apache.commons.collections.CollectionUtils;
import org.junit.Assert;
import org.junit.jupiter.api.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.List;
import java.util.Optional;

/**
 * GaussDBTJobQueueProvider测试类
 *
 */
@SpringBootTest(classes = {
    GaussDBTJobQueueProvider.class
})
@RunWith(SpringRunner.class)
class GaussDBTJobQueueProviderTest {

    @Autowired
    private GaussDBTJobQueueProvider gaussDBTJobQueueProvider;

    @MockBean
    private ResourceService resourceService;

    @Test
    public void get_customized_schedule_policy_success() {
        Mockito.when(resourceService.getBasicResourceById(any()))
            .thenReturn(Optional.ofNullable(mockProtectedResource()));
        List<JobSchedulePolicy> jobSchedulePolicies = gaussDBTJobQueueProvider.getCustomizedSchedulePolicy(mockJob());
        Assert.assertTrue(CollectionUtils.size(jobSchedulePolicies) == 1);
    }

    private Job mockJob() {
        Job job = new Job();
        job.setSourceId("test");
        job.setSourceSubType(ResourceSubTypeEnum.GAUSSDBT.getType());
        return job;
    }

    private ProtectedResource mockProtectedResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setRootUuid("rootUuid");
        return resource;
    }
}