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
package openbackup.antdb.protection.access.provider.restore;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import com.google.common.collect.ImmutableMap;

import openbackup.antdb.protection.access.common.AntDBConstants;
import openbackup.antdb.protection.access.service.AntDBInstanceService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.AgentMountTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Answers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {AntDBInstanceRestoreProvider.class})
@PowerMockIgnore( {"javax.management.*", "javax.net.ssl.*", "jdk.internal.reflect.*"})
public class AntDBInstanceRestoreProviderTest {
    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ProviderManager providerManager;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ResourceService resourceService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private AntDBInstanceService antDBInstanceService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ProtectedEnvironmentService protectedEnvironmentService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private CopyRestApi copyRestApi;

    @InjectMocks
    private AntDBInstanceRestoreProvider antDBInstanceRestoreProvider;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        antDBInstanceService = mock(AntDBInstanceService.class, Answers.RETURNS_DEEP_STUBS);
        ReflectionTestUtils.setField(antDBInstanceRestoreProvider, "antDBInstanceService", antDBInstanceService);
        protectedEnvironmentService = mock(ProtectedEnvironmentService.class, Answers.RETURNS_DEEP_STUBS);
        ReflectionTestUtils.setField(antDBInstanceRestoreProvider, "protectedEnvironmentService",
            protectedEnvironmentService);
        copyRestApi = mock(CopyRestApi.class, Answers.RETURNS_DEEP_STUBS);
        ReflectionTestUtils.setField(antDBInstanceRestoreProvider, "copyRestApi", copyRestApi);
    }

    @After
    public void tearDown() throws Exception {
    }

    @Test
    public void test_applicable_should_return_true_when_condition() throws Exception {

        // run the test
        boolean result = antDBInstanceRestoreProvider.applicable(ResourceSubTypeEnum.ANT_DB_INSTANCE.getType());

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_applicable_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        boolean result = antDBInstanceRestoreProvider.applicable(null);
    }

    @Test
    public void test_getLockResources1_should_return_not_null_when_condition() throws Exception {
        // setup
        RestoreTask task = new RestoreTask();
        TaskResource targetObject = new TaskResource();
        targetObject.setUuid("string");
        task.setTargetObject(targetObject);

        // run the test
        List<LockResourceBo> result = antDBInstanceRestoreProvider.getLockResources(task);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_getLockResources1_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        List<LockResourceBo> result = antDBInstanceRestoreProvider.getLockResources((RestoreTask) null);
    }

    @Test
    public void test_supplyRestoreTask1_should_return_not_null_when_condition() throws Exception {
        // setup
        Copy copy = new Copy();
        copy.setGeneratedBy("string");
        copy.setResourceProperties("{'version':'6.3.0','extendInfo':{'osUsername':'antdb'}}");
        when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);

        ProtectedResource resource = new ProtectedResource();
        resource.setVersion("6.3.0");
        resource.setExtendInfo(ImmutableMap.of(AntDBConstants.DB_OS_USER_KEY,"antdb"));

        when(antDBInstanceService.getResourceById(anyString())).thenReturn(resource);

        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("string");
        environment.setPort(0);
        Map dependencies = new HashMap<>();
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setVersion("string");
        list.add(protectedResource);
        dependencies.put("string", list);
        environment.setDependencies(dependencies);
        Map extendInfo = mock(Map.class);
        environment.setExtendInfo(extendInfo);
        environment.setUuid("string");
        when(protectedEnvironmentService.getEnvironmentById(anyString())).thenReturn(environment);

        RestoreTask task = new RestoreTask();
        task.setRestoreType("string");
        Map advanceParams = new HashMap<>();
        advanceParams.put("string", "string");
        task.setAdvanceParams(advanceParams);
        task.setTargetLocation(RestoreLocationEnum.NEW);
        task.setRestoreMode("string");
        task.setCopyId("string");
        TaskEnvironment targetEnv = new TaskEnvironment();
        targetEnv.setUuid("string");
        Map extendInfo1 = mock(Map.class);
        targetEnv.setExtendInfo(extendInfo1);
        task.setTargetEnv(targetEnv);
        TaskResource targetObject = new TaskResource();
        targetObject.setUuid("string");
        targetObject.setSubType(ResourceSubTypeEnum.ANT_DB_INSTANCE.getType());
        Map extendInfo2 = mock(Map.class);
        targetObject.setExtendInfo(extendInfo2);
        task.setTargetObject(targetObject);

        // run the test
        RestoreTask result = antDBInstanceRestoreProvider.supplyRestoreTask(task);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_supplyRestoreTask1_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        when(copyRestApi.queryCopyByID(anyString())).thenReturn((Copy) null);

        when(antDBInstanceService.getResourceById(anyString())).thenReturn((ProtectedResource) null);

        when(protectedEnvironmentService.getEnvironmentById(anyString())).thenReturn((ProtectedEnvironment) null);

        // run the test
        RestoreTask result = antDBInstanceRestoreProvider.supplyRestoreTask((RestoreTask) null);
    }

    @Test
    public void test_initialize_should_return_not_null_when_condition() throws Exception {
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

        RestoreTask task = new RestoreTask();
        TaskEnvironment targetEnv = new TaskEnvironment();
        targetEnv.setUuid("string");
        task.setTargetEnv(targetEnv);

        // run the test
        RestoreTask result = antDBInstanceRestoreProvider.initialize(task);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_initialize_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        when(resourceService.getResourceById(anyString())).thenReturn(null);

        ResourceConnectionCheckProvider provider = mock(ResourceConnectionCheckProvider.class);
        when(provider.checkConnection(any(ProtectedResource.class))).thenReturn(null);
        when(providerManager.findProviderOrDefault(eq(ResourceConnectionCheckProvider.class),
            any(ProtectedResource.class), any(ResourceConnectionCheckProvider.class))).thenReturn(provider);

        // run the test
        RestoreTask result = antDBInstanceRestoreProvider.initialize(null);
    }

    @Test
    public void test_postProcess1_should_void_when_condition() throws Exception {
        // setup
        RestoreTask task = new RestoreTask();
        TaskResource targetObject = new TaskResource();
        targetObject.setUuid("string");
        task.setTargetObject(targetObject);

        // run the test
        antDBInstanceRestoreProvider.postProcess(task, ProviderJobStatusEnum.PENDING);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_postProcess1_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBInstanceRestoreProvider.postProcess((RestoreTask) null, (ProviderJobStatusEnum) null);
    }

    @Test
    public void test_supplyRestoreTask1_should_return_not_null_when_condition1() throws Exception {
        // setup
        RestoreTask task = new RestoreTask();

        // run the test
        RestoreTask result = antDBInstanceRestoreProvider.supplyRestoreTask(task);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_supplyRestoreTask1_should_throws_null_pointer_exception_when_objects_is_null1() throws Exception {

        // run the test
        RestoreTask result = antDBInstanceRestoreProvider.supplyRestoreTask((RestoreTask) null);
    }

    @Test
    public void test_getLockResources1_should_return_not_null_when_condition1() throws Exception {
        // setup
        RestoreTask task = new RestoreTask();
        TaskResource targetObject = new TaskResource();
        targetObject.setUuid("string");
        task.setTargetObject(targetObject);

        // run the test
        List<LockResourceBo> result = antDBInstanceRestoreProvider.getLockResources(task);

        // verify the results
        assertNotNull(result);
    }

    @Test
    public void test_postProcess1_should_void_when_condition2() throws Exception {
        // setup
        RestoreTask task = new RestoreTask();

        // run the test
        antDBInstanceRestoreProvider.postProcess(task, ProviderJobStatusEnum.PENDING);

        // verify the results
        assertTrue(true);
    }

    @Test
    public void test_longTimeStopProcess_should_void_when_condition() throws Exception {
        // setup
        JobBo job = new JobBo();

        // run the test
        antDBInstanceRestoreProvider.longTimeStopProcess(job);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_longTimeStopProcess_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBInstanceRestoreProvider.longTimeStopProcess(null);
    }

    @Test
    public void test_restoreTaskCreationPreCheck_should_void_when_condition() throws Exception {
        // setup
        RestoreTask task = new RestoreTask();

        // run the test
        antDBInstanceRestoreProvider.restoreTaskCreationPreCheck(task);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_restoreTaskCreationPreCheck_should_throws_null_pointer_exception_when_objects_is_null()
        throws Exception {

        // run the test
        antDBInstanceRestoreProvider.restoreTaskCreationPreCheck(null);
    }

    @Test
    public void test_queryEnvironment_should_return_not_null_when_condition() throws Exception {

        // run the test
        Optional<ProtectedEnvironment> result = antDBInstanceRestoreProvider.queryEnvironment("string");

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_queryEnvironment_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        Optional<ProtectedEnvironment> result = antDBInstanceRestoreProvider.queryEnvironment(null);
    }

    @Test
    public void test_getRestoreFeature_should_return_not_null_when_condition() throws Exception {

        // run the test
        RestoreFeature result = antDBInstanceRestoreProvider.getRestoreFeature();

        // verify the results
        assertNotNull(result);
    }

    @Test
    public void test_afterSendTask_should_void_when_condition() throws Exception {
        // setup
        RestoreTask task = new RestoreTask();

        // run the test
        antDBInstanceRestoreProvider.afterSendTask(task);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_afterSendTask_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBInstanceRestoreProvider.afterSendTask(null);
    }

    @Test
    public void test_getMountType_should_return_not_null_when_condition() throws Exception {
        // setup
        RestoreTask task = new RestoreTask();

        // run the test
        Optional<AgentMountTypeEnum> result = antDBInstanceRestoreProvider.getMountType(task);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_getMountType_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        Optional<AgentMountTypeEnum> result = antDBInstanceRestoreProvider.getMountType(null);
    }

    @Test
    public void test_encryptExtendInfo_should_void_when_condition() throws Exception {
        // setup
        Map extendInfo = new HashMap<>();
        extendInfo.put("string", "string");

        // run the test
        antDBInstanceRestoreProvider.encryptExtendInfo(extendInfo);

        // verify the results
        assertEquals(1, extendInfo.size());
    }

    @Test(expected = NullPointerException.class)
    public void test_encryptExtendInfo_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBInstanceRestoreProvider.encryptExtendInfo(null);
    }
}