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
package openbackup.database.base.plugin.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.system.base.common.model.job.Job;
import org.junit.jupiter.api.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

import static org.mockito.ArgumentMatchers.any;


/**
 * GeneralDbJobQueueProvider测试类
 *
 */
@SpringBootTest(classes = {
        GeneralDbJobQueueProvider.class
})
@RunWith(SpringRunner.class)
class GeneralDbJobQueueProviderTest {

    @Autowired
    private GeneralDbJobQueueProvider generalDbJobQueueProvider;

    @MockBean
    private  ResourceService resourceService;
    @Test
    public void get_customized_schedule_policy_success_for_gbase_8a() {
        Mockito.when(resourceService.getBasicResourceById(any())).thenReturn(Optional.ofNullable(mockProtectedResource()));
        generalDbJobQueueProvider.getCustomizedSchedulePolicy(mockJob());
        Mockito.verify(resourceService, Mockito.times(2)).getBasicResourceById(any());
    }

    private Job mockJob() {
        Job job = new Job();
        job.setSourceId("test");
        job.setSourceSubType("GeneralDb");
        return job;
    }

    private ProtectedResource mockProtectedResource() {
        ProtectedResource resource = new ProtectedResource();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(GeneralDbConstant.DATABASE_TYPE_DISPLAY, "Gbase 8a");
        extendInfo.put(GeneralDbConstant.EXTEND_RELATED_HOST_IPS,"1.1.1.1");
        resource.setExtendInfo(extendInfo);
        return resource;
    }
}
