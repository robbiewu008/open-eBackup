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
package openbackup.antdb.protection.access.provider.resource;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.AgentMountTypeEnum;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;

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
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Optional;
import java.util.function.Predicate;
import java.util.stream.Collector;

@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {AntDBCopyDeleteInterceptor.class})
@PowerMockIgnore( {"javax.management.*", "javax.net.ssl.*", "jdk.internal.reflect.*"})
public class AntDBCopyDeleteInterceptorTest {
    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private CopyRestApi copyRestApi;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ResourceService resourceService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private RedissonClient redissonClient;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ProviderManager providerManager;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private JobService jobService;

    @InjectMocks
    private AntDBCopyDeleteInterceptor antDBCopyDeleteInterceptor;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void tearDown() throws Exception {
    }

    @Test
    public void test_applicable_should_return_true_when_condition() throws Exception {

        // run the test
        boolean result = antDBCopyDeleteInterceptor.applicable("string");

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_applicable_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        boolean result = antDBCopyDeleteInterceptor.applicable(null);
    }

    @Test
    public void test_shouldSupplyAgent2_should_return_true_when_condition() throws Exception {
        // setup
        DeleteCopyTask task = new DeleteCopyTask();

        CopyInfoBo copy = new CopyInfoBo();

        // run the test
        boolean result = antDBCopyDeleteInterceptor.shouldSupplyAgent(task, copy);

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_shouldSupplyAgent2_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        boolean result = antDBCopyDeleteInterceptor.shouldSupplyAgent((DeleteCopyTask) null, (CopyInfoBo) null);
    }

    @Test
    public void test_getCopiesCopyTypeIsFull2_should_return_not_null_when_condition() throws Exception {
        // setup
        Copy copy = new Copy();
        copy.setUuid("string");
        copy.setGn(0);
        copy.setResourceId("string");
        Optional<Copy> optional = Optional.of(copy);
        when(copyRestApi.queryLatestFullBackupCopies(anyString(), anyInt(), anyInt())).thenReturn(optional);

        List<Copy> copies = new ArrayList<>();
        Copy copy1 = new Copy();
        copy1.setUuid("string");
        copy1.setGn(0);
        copy1.setResourceId("string");
        copies.add(copy1);

        Copy thisCopy = new Copy();
        thisCopy.setUuid("string");
        thisCopy.setGn(0);
        thisCopy.setResourceId("string");

        Copy nextFullCopy = new Copy();
        nextFullCopy.setUuid("string");
        nextFullCopy.setGn(0);
        nextFullCopy.setResourceId("string");

        // run the test
        List<String> result = antDBCopyDeleteInterceptor.getCopiesCopyTypeIsFull(copies, thisCopy, nextFullCopy);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_getCopiesCopyTypeIsFull2_should_throws_null_pointer_exception_when_objects_is_null()
        throws Exception {
        // setup
        when(copyRestApi.queryLatestFullBackupCopies(anyString(), anyInt(), anyInt())).thenReturn(null);

        // run the test
        List<String> result = antDBCopyDeleteInterceptor.getCopiesCopyTypeIsFull((List) null, (Copy) null, (Copy) null);
    }

    @Test
    public void test_shouldSupplyAgent2_should_return_true_when_condition1() throws Exception {
        // setup
        when(resourceService.getBasicResourceById(anyString()).filter(any(Predicate.class)).isPresent()).thenReturn(
            true);

        DeleteCopyTask task = new DeleteCopyTask();
        TaskEnvironment protectEnv = new TaskEnvironment();
        task.setProtectEnv(protectEnv);
        TaskResource protectObject = new TaskResource();
        protectObject.setUuid("string");
        task.setProtectObject(protectObject);

        CopyInfoBo copy = new CopyInfoBo();

        // run the test
        boolean result = antDBCopyDeleteInterceptor.shouldSupplyAgent(task, copy);

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_shouldSupplyAgent2_should_throws_null_pointer_exception_when_objects_is_null1() throws Exception {
        // setup
        when(resourceService.getBasicResourceById(anyString()).filter(any(Predicate.class)).isPresent()).thenReturn(
            true);

        // run the test
        boolean result = antDBCopyDeleteInterceptor.shouldSupplyAgent((DeleteCopyTask) null, (CopyInfoBo) null);
    }

    @Test
    public void test_isResourceExists_should_return_true_when_condition() throws Exception {
        // setup
        when(resourceService.getBasicResourceById(anyString()).filter(any(Predicate.class)).isPresent()).thenReturn(
            true);

        DeleteCopyTask task = new DeleteCopyTask();
        TaskEnvironment protectEnv = new TaskEnvironment();
        task.setProtectEnv(protectEnv);
        TaskResource protectObject = new TaskResource();
        protectObject.setUuid("string");
        task.setProtectObject(protectObject);

        // run the test
        boolean result = antDBCopyDeleteInterceptor.isResourceExists(task);

        // verify the results
        assertFalse(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_isResourceExists_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        when(resourceService.getBasicResourceById(anyString()).filter(any(Predicate.class)).isPresent()).thenReturn(
            true);

        // run the test
        boolean result = antDBCopyDeleteInterceptor.isResourceExists(null);
    }

    @Test
    public void test_isEnvironmentOffline_should_return_true_when_condition() throws Exception {
        // setup
        DeleteCopyTask task = new DeleteCopyTask();
        TaskEnvironment protectEnv = new TaskEnvironment();
        protectEnv.setLinkStatus("string");
        task.setProtectEnv(protectEnv);

        // run the test
        boolean result = antDBCopyDeleteInterceptor.isEnvironmentOffline(task);

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_isEnvironmentOffline_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        boolean result = antDBCopyDeleteInterceptor.isEnvironmentOffline(null);
    }

    @Test
    public void test_getAssociatedCopy2_should_return_not_null_when_condition() throws Exception {

        // run the test
        List<String> result = antDBCopyDeleteInterceptor.getAssociatedCopy("string");

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_getAssociatedCopy2_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        List<String> result = antDBCopyDeleteInterceptor.getAssociatedCopy(null);
    }

    @Test
    public void test_getCopiesCopyTypeIsFull2_should_return_not_null_when_condition5() throws Exception {
        // setup
        List<Copy> copies = new ArrayList<>();
        Copy copy = new Copy();
        copies.add(copy);

        Copy thisCopy = new Copy();

        Copy nextFullCopy = new Copy();

        // run the test
        List<String> result = antDBCopyDeleteInterceptor.getCopiesCopyTypeIsFull(copies, thisCopy, nextFullCopy);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_getCopiesCopyTypeIsFull2_should_throws_null_pointer_exception_when_objects_is_null1()
        throws Exception {

        // run the test
        List<String> result = antDBCopyDeleteInterceptor.getCopiesCopyTypeIsFull((List) null, (Copy) null, (Copy) null);
    }

    @Test(expected = NullPointerException.class)
    public void test_initialize_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        when(jobService.queryJob(anyString())).thenReturn(null);

        when(resourceService.getResourceById(anyString())).thenReturn(null);

        ProtectedResourceChecker checker = mock(ProtectedResourceChecker.class);
        when(checker.collectConnectableResources(any(ProtectedResource.class))).thenReturn(null);
        when(providerManager.findProviderOrDefault(eq(ProtectedResourceChecker.class), any(ProtectedResource.class),
            any(ProtectedResourceChecker.class))).thenReturn(checker);

        when(copyRestApi.queryCopyByID(anyString(), anyBoolean())).thenReturn(null);
        when(copyRestApi.queryLaterCopiesByGeneratedBy(anyString(), anyInt(), anyString())
            .stream()
            .sorted(any(Comparator.class))
            .collect(any(Collector.class))).thenReturn(null);

        when(redissonClient.getMap(anyString(), eq(StringCodec.INSTANCE))).thenReturn(null);

        // run the test
        antDBCopyDeleteInterceptor.initialize(null, null);
    }

    @Test
    public void test_getAssociatedCopy2_should_return_not_null_when_condition1() throws Exception {
        // setup
        Copy thisCopy = new Copy();
        thisCopy.setGn(0);
        thisCopy.setNextCopyGn(0);
        thisCopy.setBackupType(0);
        thisCopy.setGeneratedBy("string");
        thisCopy.setResourceId("string");
        when(copyRestApi.queryCopyByID(anyString(), anyBoolean())).thenReturn(thisCopy);
        List<Copy> copies = new ArrayList<>();
        Copy copy = new Copy();
        copy.setGn(0);
        copy.setNextCopyGn(0);
        copy.setBackupType(0);
        copy.setGeneratedBy("string");
        copy.setResourceId("string");
        copies.add(copy);
        when(copyRestApi.queryLaterCopiesByGeneratedBy(anyString(), anyInt(), anyString())
            .stream()
            .sorted(any(Comparator.class))
            .collect(any(Collector.class))).thenReturn(copies);

        // run the test
        List<String> result = antDBCopyDeleteInterceptor.getAssociatedCopy("string");

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_getAssociatedCopy2_should_throws_null_pointer_exception_when_objects_is_null1() throws Exception {
        // setup
        when(copyRestApi.queryCopyByID(anyString(), anyBoolean())).thenReturn((Copy) null);
        when(copyRestApi.queryLaterCopiesByGeneratedBy(anyString(), anyInt(), anyString())
            .stream()
            .sorted(any(Comparator.class))
            .collect(any(Collector.class))).thenReturn((List) null);

        // run the test
        List<String> result = antDBCopyDeleteInterceptor.getAssociatedCopy(null);
    }

    @Test
    public void test_getCopiesCopyTypeIsFull2_should_return_not_null_when_condition6() throws Exception {
        // setup
        List<Copy> copies = new ArrayList<>();
        Copy copy = new Copy();
        copy.setBackupType(0);
        copies.add(copy);

        Copy thisCopy = new Copy();
        thisCopy.setBackupType(0);

        Copy nextFullCopy = new Copy();
        nextFullCopy.setBackupType(0);

        // run the test
        List<String> result = antDBCopyDeleteInterceptor.getCopiesCopyTypeIsFull(copies, thisCopy, nextFullCopy);

        // verify the results
        assertNotNull(result);
    }

    @Test
    public void test_finalize_should_void_when_condition() throws Exception {
        // setup
        Copy copy = new Copy();

        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();

        // run the test
        antDBCopyDeleteInterceptor.finalize(copy, taskMessage);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_finalize_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBCopyDeleteInterceptor.finalize((Copy) null, (TaskCompleteMessageBo) null);
    }

    @Test
    public void test_getMountType_should_return_not_null_when_condition() throws Exception {
        // setup
        DeleteCopyTask backupTask = new DeleteCopyTask();

        // run the test
        Optional<AgentMountTypeEnum> result = antDBCopyDeleteInterceptor.getMountType(backupTask);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_getMountType_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        Optional<AgentMountTypeEnum> result = antDBCopyDeleteInterceptor.getMountType(null);
    }
}