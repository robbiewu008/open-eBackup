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
package openbackup.antdb.protection.access.service.impl;

import static org.junit.Assert.assertNotNull;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.service.InstanceProtectionService;

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
import java.util.List;
import java.util.function.Supplier;

@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {AntDBInstanceServiceImpl.class})
@PowerMockIgnore( {"javax.management.*", "javax.net.ssl.*", "jdk.internal.reflect.*"})
public class AntDBInstanceServiceImplTest {
    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ResourceService resourceService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private InstanceProtectionService instanceProtectionService;

    @InjectMocks
    private AntDBInstanceServiceImpl antDBInstanceServiceImpl;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        resourceService = mock(ResourceService.class, Answers.RETURNS_DEEP_STUBS);
        ReflectionTestUtils.setField(antDBInstanceServiceImpl, "resourceService", resourceService);
        instanceProtectionService = mock(InstanceProtectionService.class, Answers.RETURNS_DEEP_STUBS);
        ReflectionTestUtils.setField(antDBInstanceServiceImpl, "instanceProtectionService", instanceProtectionService);
    }

    @After
    public void tearDown() throws Exception {
    }

    @Test
    public void test_getResourceById_should_return_not_null_when_condition() throws Exception {
        // setup
        ProtectedResource protectedResource = new ProtectedResource();
        when(resourceService.getResourceById(anyString()).orElseThrow(any(Supplier.class))).thenReturn(
            protectedResource);

        // run the test
        ProtectedResource result = antDBInstanceServiceImpl.getResourceById("string");

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_getResourceById_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        when(resourceService.getResourceById(anyString()).orElseThrow(any(Supplier.class))).thenReturn(null);

        // run the test
        ProtectedResource result = antDBInstanceServiceImpl.getResourceById(null);
    }

    @Test
    public void test_getEnvNodesByInstanceResource_should_return_not_null_when_condition() throws Exception {
        // setup
        List<TaskEnvironment> list = new ArrayList<>();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        list.add(taskEnvironment);
        when(instanceProtectionService.extractEnvNodesByClusterInstance(any(ProtectedResource.class))).thenReturn(list);
        List<TaskEnvironment> list1 = new ArrayList<>();
        TaskEnvironment taskEnvironment1 = new TaskEnvironment();
        list1.add(taskEnvironment1);
        when(instanceProtectionService.extractEnvNodesBySingleInstance(any(ProtectedResource.class))).thenReturn(list1);

        ProtectedResource instanceResource = new ProtectedResource();
        instanceResource.setSubType("string");

        // run the test
        List<TaskEnvironment> result = antDBInstanceServiceImpl.getEnvNodesByInstanceResource(instanceResource);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_getEnvNodesByInstanceResource_should_throws_null_pointer_exception_when_objects_is_null()
        throws Exception {
        // setup
        when(instanceProtectionService.extractEnvNodesByClusterInstance(any(ProtectedResource.class))).thenReturn(null);
        when(instanceProtectionService.extractEnvNodesBySingleInstance(any(ProtectedResource.class))).thenReturn(null);

        // run the test
        List<TaskEnvironment> result = antDBInstanceServiceImpl.getEnvNodesByInstanceResource(null);
    }

    @Test
    public void test_getAgentsByInstanceResource_should_return_not_null_when_condition() throws Exception {
        // setup
        List<TaskEnvironment> list = new ArrayList<>();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        list.add(taskEnvironment);
        when(instanceProtectionService.extractEnvNodesByClusterInstance(any(ProtectedResource.class))).thenReturn(list);
        List<TaskEnvironment> list1 = new ArrayList<>();
        TaskEnvironment taskEnvironment1 = new TaskEnvironment();
        list1.add(taskEnvironment1);
        when(instanceProtectionService.extractEnvNodesBySingleInstance(any(ProtectedResource.class))).thenReturn(list1);

        ProtectedResource instanceResource = new ProtectedResource();
        instanceResource.setSubType("string");

        // run the test
        List<Endpoint> result = antDBInstanceServiceImpl.getAgentsByInstanceResource(instanceResource);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_getAgentsByInstanceResource_should_throws_null_pointer_exception_when_objects_is_null()
        throws Exception {
        // setup
        when(instanceProtectionService.extractEnvNodesByClusterInstance(any(ProtectedResource.class))).thenReturn(null);
        when(instanceProtectionService.extractEnvNodesBySingleInstance(any(ProtectedResource.class))).thenReturn(null);

        // run the test
        List<Endpoint> result = antDBInstanceServiceImpl.getAgentsByInstanceResource(null);
    }
}