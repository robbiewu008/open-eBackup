/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
 * @author c00826511
 * @since 2024-06-13
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
