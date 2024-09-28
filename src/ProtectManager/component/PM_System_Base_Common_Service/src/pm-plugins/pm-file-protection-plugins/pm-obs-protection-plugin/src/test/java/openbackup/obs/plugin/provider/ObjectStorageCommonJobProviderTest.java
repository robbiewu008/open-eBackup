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
package openbackup.obs.plugin.provider;

import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Optional;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
public class ObjectStorageCommonJobProviderTest {
    private ObjectStorageCommonJobProvider objectStorageCommonJobProvider;

    private ResourceService resourceService;

    @Before
    public void init() {
        resourceService = PowerMockito.mock(ResourceService.class);
        objectStorageCommonJobProvider = new ObjectStorageCommonJobProvider(resourceService);
    }

    @Test
    public void test_applicable() {
        boolean result = objectStorageCommonJobProvider.applicable(ResourceSubTypeEnum.OBJECT_SET.getType());
        Assert.assertTrue(result);

        boolean result2 = objectStorageCommonJobProvider.applicable(ResourceSubTypeEnum.OBJECT_STORAGE.getType());
        Assert.assertFalse(result2);
    }

    @Test
    public void test_intercept() {
        Job job = new Job();
        job.setSourceId("sourceID");
        job.setType(JobTypeEnum.BACKUP.getValue());
        job.setExtendStr("{}");

        ProtectedEnvironment env = new ProtectedEnvironment();
        env.setExtendInfoByKey("storageType", "2");

        ProtectedResource resource = new ProtectedResource();
        resource.setEnvironment(env);

        PowerMockito.when(resourceService.getBasicResourceById(anyBoolean(), anyBoolean(), anyString()))
            .thenReturn(Optional.of(resource));

        objectStorageCommonJobProvider.intercept(job);
        Assert.assertEquals("{\"storageType\":\"2\"}", job.getExtendStr());
    }
}
