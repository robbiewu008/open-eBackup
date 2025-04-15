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
package openbackup.antdb.protection.access.provider.backup;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.AgentMountTypeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Answers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.OptionalInt;
import java.util.function.Supplier;

@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {AntDBBackupInterceptorProvider.class})
@PowerMockIgnore( {"javax.management.*", "javax.net.ssl.*", "jdk.internal.reflect.*"})
public class AntDBBackupInterceptorProviderTest {
    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ResourceService resourceService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ProviderManager providerManager;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private RedissonClient redissonClient;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private DataBaseAgentSelector dataBaseAgentSelector;

    @InjectMocks
    private AntDBBackupInterceptorProvider antDBBackupInterceptorProvider;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        resourceService = mock(ResourceService.class, Answers.RETURNS_DEEP_STUBS);
        ReflectionTestUtils.setField(antDBBackupInterceptorProvider, "resourceService", resourceService);
    }

    @After
    public void tearDown() throws Exception {
    }

    @Test
    public void test_supplyBackupTask1_should_return_not_null_when_condition() throws Exception {
        // setup
        ProtectedResource protectedResource = new ProtectedResource();
        Map dependencies = new HashMap();
        dependencies.put(DatabaseConstants.CHILDREN, Arrays.asList(new ProtectedResource()));
        protectedResource.setDependencies(dependencies);
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(Optional.ofNullable(protectedResource));

        BackupTask backupTask = new BackupTask();
        backupTask.setTaskId("string");
        backupTask.setBackupType("string");
        TaskResource protectObject = new TaskResource();
        protectObject.setUuid("string");
        protectObject.setSubType("string");
        backupTask.setProtectObject(protectObject);
        TaskEnvironment protectEnv = new TaskEnvironment();
        Map extendInfo = new HashMap<>();
        extendInfo.put("string", "string");
        protectEnv.setExtendInfo(extendInfo);
        backupTask.setProtectEnv(protectEnv);
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository storageRepository = new StorageRepository();
        backupTask.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);
        repositories.add(storageRepository);
        backupTask.setRepositories(repositories);

        // run the test
        BackupTask result = antDBBackupInterceptorProvider.supplyBackupTask(backupTask);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_supplyBackupTask1_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        when(resourceService.getResourceById(anyString()).orElseThrow(any(Supplier.class))).thenReturn(null);

        // run the test
        BackupTask result = antDBBackupInterceptorProvider.supplyBackupTask((BackupTask) null);
    }

    @Test
    public void test_supplyNodes1_should_void_when_condition() throws Exception {
        // setup
        ProtectedResource protectedResource = new ProtectedResource();
        Map extendInfo = new HashMap<>();
        extendInfo.put("string", "string");
        protectedResource.setExtendInfo(extendInfo);
        Authentication auth = new Authentication();
        protectedResource.setAuth(auth);
        Map dependencies = new HashMap();
        ProtectedResource child = new ProtectedResource();
        Map childDependencies = new HashMap();
        childDependencies.put(DatabaseConstants.AGENTS,Arrays.asList(new ProtectedResource()));
        child.setDependencies(childDependencies);
        dependencies.put(DatabaseConstants.CHILDREN, Arrays.asList(child));
        protectedResource.setDependencies(dependencies);
        PowerMockito.when(resourceService.getResourceById(any()))
            .thenReturn(Optional.ofNullable(protectedResource));

        BackupTask backupTask = new BackupTask();
        TaskResource protectObject = new TaskResource();
        protectObject.setUuid("string");
        protectObject.setSubType("string");
        backupTask.setProtectObject(protectObject);
        TaskEnvironment protectEnv = new TaskEnvironment();
        backupTask.setProtectEnv(protectEnv);

        // run the test
        antDBBackupInterceptorProvider.supplyNodes(backupTask);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_supplyNodes1_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        when(resourceService.getResourceById(anyString()).orElseThrow(any(Supplier.class))).thenReturn(null);

        // run the test
        antDBBackupInterceptorProvider.supplyNodes((BackupTask) null);
    }

    @Test
    public void test_supplyAgent1_should_void_when_condition() throws Exception {
        // setup
        ProtectedResource protectedResource = new ProtectedResource();
        Map extendInfo = new HashMap<>();
        extendInfo.put("string", "string");
        protectedResource.setExtendInfo(extendInfo);
        Authentication auth = new Authentication();
        protectedResource.setAuth(auth);
        when(resourceService.getResourceById(anyString()).orElseThrow(any(Supplier.class))).thenReturn(
            protectedResource);

        BackupTask backupTask = new BackupTask();
        TaskResource protectObject = new TaskResource();
        protectObject.setUuid("string");
        protectObject.setSubType("string");
        backupTask.setProtectObject(protectObject);

        // run the test
        antDBBackupInterceptorProvider.supplyAgent(backupTask);

        // verify the results
        assertNotNull(backupTask.getAgents());
    }

    @Test(expected = NullPointerException.class)
    public void test_supplyAgent1_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        when(resourceService.getResourceById(anyString()).orElseThrow(any(Supplier.class))).thenReturn(null);

        // run the test
        antDBBackupInterceptorProvider.supplyAgent((BackupTask) null);
    }

    @Test
    public void test_checkConnention1_should_void_when_condition() throws Exception {
        // setup
        BackupTask backupTask = new BackupTask();

        // run the test
        antDBBackupInterceptorProvider.checkConnention(backupTask);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_checkConnention1_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBBackupInterceptorProvider.checkConnention((BackupTask) null);
    }

    @Test
    public void test_applicable_should_return_true_when_condition() throws Exception {

        // run the test
        boolean result = antDBBackupInterceptorProvider.applicable(ResourceSubTypeEnum.ANT_DB_INSTANCE.getType());

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_applicable_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        boolean result = antDBBackupInterceptorProvider.applicable(null);
    }

    @Test(expected = NullPointerException.class)
    public void test_initialize_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        when(resourceService.getResourceById(anyString())).thenReturn(null);

        when(redissonClient.getMap(anyString(), eq(StringCodec.INSTANCE))).thenReturn(null);

        when(dataBaseAgentSelector.getSelectedAgents(any(AgentSelectParam.class))).thenReturn(null);

        ResourceConnectionCheckProvider provider = mock(ResourceConnectionCheckProvider.class);
        when(provider.checkConnection(any(ProtectedResource.class))).thenReturn(null);
        when(providerManager.findProviderOrDefault(eq(ResourceConnectionCheckProvider.class),
            any(ProtectedResource.class), any(ResourceConnectionCheckProvider.class))).thenReturn(provider);

        // run the test
        BackupTask result = antDBBackupInterceptorProvider.initialize(null);
    }

    @Test
    public void test_supplyAgent1_should_void_when_condition1() throws Exception {
        // setup
        List<Endpoint> list = new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        list.add(endpoint);
        when(dataBaseAgentSelector.getSelectedAgents(any(AgentSelectParam.class))).thenReturn(list);

        BackupTask backupTask = new BackupTask();
        TaskResource protectObject = new TaskResource();
        backupTask.setProtectObject(protectObject);
        TaskEnvironment protectEnv = new TaskEnvironment();
        backupTask.setProtectEnv(protectEnv);
        Map advanceParams = new HashMap<>();
        advanceParams.put("string", "string");
        backupTask.setAdvanceParams(advanceParams);

        // run the test
        antDBBackupInterceptorProvider.supplyAgent(backupTask);

        // verify the results
        assertNotNull(backupTask.getAgents());
    }

    @Test(expected = NullPointerException.class)
    public void test_supplyAgent1_should_throws_null_pointer_exception_when_objects_is_null1() throws Exception {
        // setup
        when(dataBaseAgentSelector.getSelectedAgents(any(AgentSelectParam.class))).thenReturn(null);

        // run the test
        antDBBackupInterceptorProvider.supplyAgent((BackupTask) null);
    }

    @Test
    public void test_supplyBackupTask1_should_return_not_null_when_condition1() throws Exception {
        // setup
        BackupTask backupTask = new BackupTask();

        // run the test
        BackupTask result = antDBBackupInterceptorProvider.supplyBackupTask(backupTask);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_supplyBackupTask1_should_throws_null_pointer_exception_when_objects_is_null1() throws Exception {

        // run the test
        BackupTask result = antDBBackupInterceptorProvider.supplyBackupTask((BackupTask) null);
    }

    @Test
    public void test_checkConnention1_should_void_when_condition1() throws Exception {
        // setup
        ProtectedResource protectedResource = new ProtectedResource();
        Optional<ProtectedResource> optional = Optional.of(protectedResource);
        when(resourceService.getResourceById(anyString())).thenReturn(optional);

        ResourceConnectionCheckProvider provider = mock(ResourceConnectionCheckProvider.class);
        ResourceCheckContext resourceCheckContext = new ResourceCheckContext();
        List<ActionResult> actionResults = new ArrayList<>();
        ActionResult actionResult = new ActionResult();
        actionResults.add(actionResult);
        resourceCheckContext.setActionResults(actionResults);
        when(provider.checkConnection(any(ProtectedResource.class))).thenReturn(resourceCheckContext);
        when(providerManager.findProviderOrDefault(eq(ResourceConnectionCheckProvider.class),
            any(ProtectedResource.class), any(ResourceConnectionCheckProvider.class))).thenReturn(provider);

        BackupTask backupTask = new BackupTask();
        TaskResource protectObject = new TaskResource();
        protectObject.setUuid("string");
        backupTask.setProtectObject(protectObject);

        // run the test
        antDBBackupInterceptorProvider.checkConnention(backupTask);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_checkConnention1_should_throws_null_pointer_exception_when_objects_is_null1() throws Exception {
        // setup
        when(resourceService.getResourceById(anyString())).thenReturn(null);

        ResourceConnectionCheckProvider provider = mock(ResourceConnectionCheckProvider.class);
        when(provider.checkConnection(any(ProtectedResource.class))).thenReturn((ResourceCheckContext) null);
        when(providerManager.findProviderOrDefault(eq(ResourceConnectionCheckProvider.class),
            any(ProtectedResource.class), any(ResourceConnectionCheckProvider.class))).thenReturn(provider);

        // run the test
        antDBBackupInterceptorProvider.checkConnention((BackupTask) null);
    }

    @Test
    public void test_supplyNodes1_should_void_when_condition1() throws Exception {
        // setup
        BackupTask backupTask = new BackupTask();
        TaskEnvironment protectEnv = new TaskEnvironment();
        backupTask.setProtectEnv(protectEnv);
        List<Endpoint> agents = new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        agents.add(endpoint);
        backupTask.setAgents(agents);

        // run the test
        antDBBackupInterceptorProvider.supplyNodes(backupTask);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_supplyNodes1_should_throws_null_pointer_exception_when_objects_is_null1() throws Exception {

        // run the test
        antDBBackupInterceptorProvider.supplyNodes((BackupTask) null);
    }

    @Test
    public void test_finalize_should_void_when_condition() throws Exception {
        // setup
        PostBackupTask postBackupTask = new PostBackupTask();

        // run the test
        antDBBackupInterceptorProvider.finalize(postBackupTask);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_finalize_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBBackupInterceptorProvider.finalize((PostBackupTask) null);
    }

    @Test
    public void test_isSupportDataAndLogParallelBackup_should_return_true_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        boolean result = antDBBackupInterceptorProvider.isSupportDataAndLogParallelBackup(resource);

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_isSupportDataAndLogParallelBackup_should_throws_null_pointer_exception_when_objects_is_null()
        throws Exception {

        // run the test
        boolean result = antDBBackupInterceptorProvider.isSupportDataAndLogParallelBackup(null);
    }

    @Test
    public void test_transferBackupType_should_return_not_null_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        BackupTypeConstants result = antDBBackupInterceptorProvider.transferBackupType(BackupTypeConstants.FULL,
            resource);

        // verify the results
        assertEquals(BackupTypeConstants.FULL, result);
    }

    @Test(expected = NullPointerException.class)
    public void test_transferBackupType_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        BackupTypeConstants result = antDBBackupInterceptorProvider.transferBackupType(null, null);
    }

    @Test
    public void test_getLockResources_should_return_not_null_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        List<LockResourceBo> result = antDBBackupInterceptorProvider.getLockResources(resource);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_getLockResources_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        List<LockResourceBo> result = antDBBackupInterceptorProvider.getLockResources(null);
    }

    @Test
    public void test_getMountType_should_return_not_null_when_condition() throws Exception {
        // setup
        BackupTask backupTask = new BackupTask();

        // run the test
        Optional<AgentMountTypeEnum> result = antDBBackupInterceptorProvider.getMountType(backupTask);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_getMountType_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        Optional<AgentMountTypeEnum> result = antDBBackupInterceptorProvider.getMountType(null);
    }
}