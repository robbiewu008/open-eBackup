/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.obs.plugin.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.obs.plugin.service.ObjectStorageAgentService;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import openbackup.system.base.common.constants.LocalStorageInfoRes;
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
import java.util.Optional;

/**
 * ObjectStorageBackupProvider Test
 *
 * @author c30035089
 * @since 2023-11-21
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(EnvironmentLinkStatusHelper.class)
public class ObjectStorageBackupProviderTest {
    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ResourceService resourceService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private EncryptorService encryptorService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ObjectStorageAgentService agentService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private LocalStorageService localStorageService;

    @InjectMocks
    private ObjectStorageBackupProvider objectStorageProvider;

    @Before
    public void init() {
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    @Test
    public void test_applicable() {
        boolean result = objectStorageProvider.applicable(ResourceSubTypeEnum.OBJECT_SET.getType());
        Assert.assertTrue(result);

        boolean result2 = objectStorageProvider.applicable(ResourceSubTypeEnum.OBJECT_STORAGE.getType());
        Assert.assertFalse(result2);
    }

    @Test
    public void test_intercept_success() {
        PowerMockito.when(resourceService.getBasicResourceById(anyString()))
            .thenReturn(Optional.of(createProtectedResource()));
        PowerMockito.when(encryptorService.encrypt(anyString())).thenReturn("string");
        PowerMockito.when(encryptorService.decrypt(anyString())).thenReturn("string");
        PowerMockito.when(agentService.getObjectStorageEndpoint(anyString())).thenReturn(mockEndPoint());
        PowerMockito.when(agentService.checkConnection(any())).thenReturn(mockResult());
        LocalStorageInfoRes localStorageInfoRes = new LocalStorageInfoRes();
        localStorageInfoRes.setEsn("2222");
        PowerMockito.when(localStorageService.getStorageInfo()).thenReturn(localStorageInfoRes);

        BackupTask backupTask = new BackupTask();
        backupTask.setRequestId("1111");
        backupTask.setTaskId("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        backupTask.setCopyId("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        backupTask.setBackupType("fullBackup");
        TaskEnvironment environment = mockEnvironment();
        backupTask.setProtectEnv(environment);
        TaskResource protectedResource = mockProtectObject();
        backupTask.setProtectObject(protectedResource);
        backupTask.setRepositories(mockRepositorys());
        objectStorageProvider.initialize(backupTask);
        Assert.assertEquals(backupTask.getRepositories().size(), 3);
    }

    @Test
    public void test_get_lock_resources() {
        ProtectedResource protectedResource = createProtectedResource();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("bucketList",
            "[{\"name\":\"testOBS1\",\"prefix\":[\"1\",\"12\",\"123\"]},{\"name\":\"testOBSBS\",\"prefix\":[\"a\",\"ab\",\"abc\"]}]");
        extendInfo.put("next_backup_change_cause", "bigdata_plugin_to_full_label");
        protectedResource.setExtendInfo(extendInfo);
        List<LockResourceBo> lockResources = objectStorageProvider.getLockResources(protectedResource);
        Assert.assertEquals(2, lockResources.size());
    }

    public static TaskEnvironment mockEnvironment() {
        TaskEnvironment environment = new TaskEnvironment();
        environment.setUuid("env_id");
        environment.setName("env_name");
        environment.setEndpoint("127.0.0.1");
        Map<String, String> authExtendInfo = new HashMap<>();
        Authentication auth = new Authentication();
        auth.setAuthKey("test_key");
        auth.setAuthPwd("test_pwd");
        auth.setExtendInfo(authExtendInfo);
        environment.setAuth(auth);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("InternalAgentConnection_6278fe77-af59-419e-b373-d5d075f3f80f", "true");
        environment.setExtendInfo(extendInfo);
        return environment;
    }

    public static List<Endpoint> mockEndPoint() {
        List<Endpoint> endpoints = new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        endpoint.setId("1111");
        endpoint.setPort(11);
        endpoints.add(endpoint);
        return endpoints;
    }

    public static ActionResult[] mockResult() {
        ActionResult[] results = new ActionResult[1];
        ActionResult result = new ActionResult();
        result.setCode(0);
        result.setMessage("err");
        results[0] = result;
        return results;
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
            "[{\"name\":\"testOBS1\",\"prefix\":[\"1\",\"12\",\"123\"]},{\"name\":\"testOBSBS\",\"prefix\":[\"a\",\"ab\",\"abc\"]}]");
        extendInfo.put("next_backup_change_cause", "bigdata_plugin_to_full_label");
        protectedResource.setExtendInfo(extendInfo);
        return protectedResource;
    }

    public static List<StorageRepository> mockRepositorys() {
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository repository = new StorageRepository();
        repository.setId("111");
        repository.setRole(1);
        StorageRepository repository2 = new StorageRepository();
        repository2.setId("222");
        repository2.setRole(1);
        repositories.add(repository);
        repositories.add(repository2);
        return repositories;
    }

    public static ProtectedResource createProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setRootUuid("111");
        protectedResource.setType("1");
        return protectedResource;
    }

}
