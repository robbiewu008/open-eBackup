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

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.obs.plugin.service.ObjectStorageAgentService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Answers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * ObjectStorageBackupProvider Test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(EnvironmentLinkStatusHelper.class)
public class ObjectStorageRestoreProviderTest {
    @Mock()
    private CopyRestApi copyRestApi;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private EncryptorService encryptorService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ObjectStorageAgentService agentService;

    @InjectMocks
    private ObjectStorageRestoreProvider objectStorageProvider;

    @Before
    public void init() {
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    @Test
    public void test_applicable() {
        boolean isObjectSet = objectStorageProvider.applicable(ResourceSubTypeEnum.OBJECT_SET.getType());
        Assert.assertTrue(isObjectSet);

        boolean isObjectStorage = objectStorageProvider.applicable(ResourceSubTypeEnum.OBJECT_STORAGE.getType());
        Assert.assertTrue(isObjectStorage);
    }

    @Test
    public void test_intercept_success() {
        PowerMockito.when(encryptorService.encrypt(anyString())).thenReturn("string");
        PowerMockito.when(agentService.getObjectStorageEndpoint(anyString())).thenReturn(mockEndPoint());
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(mockCopy());
        // PowerMockito.when(agentService.checkConnection(any())).thenReturn(mockResult());
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setRequestId("1111");
        restoreTask.setTaskId("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        restoreTask.setCopyId("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        TaskEnvironment environment = mockEnvironment();
        restoreTask.setTargetEnv(environment);
        TaskResource protectedResource = mockProtectObject();
        restoreTask.setTargetObject(protectedResource);

        objectStorageProvider.initialize(restoreTask);
        Assert.assertNotNull(restoreTask.getRestoreMode());
    }

    @Test
    public void test_get_lock_resources() {
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(mockCopy());
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setCopyId("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        restoreTask.setTargetEnv(mockEnvironment());
        List<LockResourceBo> lockResources = objectStorageProvider.getLockResources(restoreTask);
        Assert.assertEquals(1, lockResources.size());
        Assert.assertEquals("bucketwei_env_id", lockResources.get(0).getId());
    }

    @Test
    public void test_get_lock_resources2() {
        Map<String, String> adv = new HashMap<>();
        adv.put("bucketName", "bucketwei");
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setAdvanceParams(adv);
        restoreTask.setTargetEnv(mockEnvironment());
        List<LockResourceBo> lockResources = objectStorageProvider.getLockResources(restoreTask);
        Assert.assertEquals(1, lockResources.size());
        Assert.assertEquals("bucketwei_env_id", lockResources.get(0).getId());
    }

    private Copy mockCopy() {
        Copy copy = new Copy();
        copy.setResourceProperties(
            "{\"extendInfo\":{\"bucketList\":\"[{\\\"name\\\":\\\"bucketwei\\\",\\\"prefix\\\":[\\\"aa/\\\"]}]\"}}");
        return copy;
    }

    public static TaskEnvironment mockEnvironment() {
        TaskEnvironment environment = new TaskEnvironment();
        environment.setUuid("env_id");
        environment.setName("env_name");
        environment.setType("Storage");
        environment.setSubType("ObjectStorage");
        environment.setEndpoint("127.0.0.1");
        Map<String, String> authExtendInfo = new HashMap<>();
        Authentication auth = new Authentication();
        auth.setAuthKey("test_key");
        auth.setAuthPwd("test_pwd");
        authExtendInfo.put("endpoint", "33.16.7.10");
        authExtendInfo.put("proxyEnable", "1");
        authExtendInfo.put("useHttps", "0");
        authExtendInfo.put("sk", "xxxxxxx");
        authExtendInfo.put("ak", "ddddddddd");
        authExtendInfo.put("proxyHostName", "http://192.168.194.168:8081");
        auth.setExtendInfo(authExtendInfo);
        environment.setAuth(auth);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("InternalAgentConnection_6278fe77-af59-419e-b373-d5d075f3f80f", "true");
        extendInfo.put("agents", "208d062b-02d4-4225-9048-3be19ee40340");
        extendInfo.put("storageType", "2");
        environment.setExtendInfo(extendInfo);
        return environment;
    }

    public static TaskResource mockProtectObject() {
        TaskResource protectedResource = new TaskResource();
        protectedResource.setUuid("env_id");
        protectedResource.setName("env_name");
        protectedResource.setSubType("Storage");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("next_backup_type", "true");
        extendInfo.put("storageType", "2");
        extendInfo.put("bucketList",
            "[{\\\"name\\\":\\\"testOBS1\\\",\\\"prefix\\\":[\\\"1\\\",\\\"12\\\",\\\"123\\\"]},{\\\"name\\\":\\\"testOBSBS\\\",\\\"prefix\\\":[\\\"a\\\",\\\"ab\\\",\\\"abc\\\"]}]");
        extendInfo.put("next_backup_change_cause", "bigdata_plugin_to_full_label");
        protectedResource.setExtendInfo(extendInfo);
        return protectedResource;
    }

    public static List<Endpoint> mockEndPoint() {
        List<Endpoint> endpoints = new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        endpoint.setId("1111");
        endpoint.setPort(11);
        endpoints.add(endpoint);
        return endpoints;
    }
}
