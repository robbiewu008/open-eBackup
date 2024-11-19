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

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.service.InstanceResourceService;

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

@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {AntDBClusterInstanceConnectionChecker.class})
@PowerMockIgnore( {"javax.management.*", "javax.net.ssl.*", "jdk.internal.reflect.*"})
public class AntDBClusterInstanceConnectionCheckerTest {
    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ProtectedEnvironmentService protectedEnvironmentService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private InstanceResourceService instanceResourceService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ProtectedEnvironmentRetrievalsService environmentRetrievalsService;

    @InjectMocks
    private AntDBClusterInstanceConnectionChecker antDBClusterInstanceConnectionChecker;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        protectedEnvironmentService = mock(ProtectedEnvironmentService.class, Answers.RETURNS_DEEP_STUBS);
        ReflectionTestUtils.setField(antDBClusterInstanceConnectionChecker, "protectedEnvironmentService",
            protectedEnvironmentService);
        instanceResourceService = mock(InstanceResourceService.class, Answers.RETURNS_DEEP_STUBS);
        ReflectionTestUtils.setField(antDBClusterInstanceConnectionChecker, "instanceResourceService",
            instanceResourceService);
    }

    @After
    public void tearDown() throws Exception {
    }

    @Test
    public void test_collectConnectableResources1_should_return_not_null_when_condition() throws Exception {
        // setup
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("string");
        when(protectedEnvironmentService.getEnvironmentById(anyString())).thenReturn(environment);

        ProtectedResource resource = new ProtectedResource();
        Map extendInfo = new HashMap<>();
        extendInfo.put("string", "string");
        resource.setExtendInfo(extendInfo);
        ProtectedEnvironment environment1 = new ProtectedEnvironment();
        environment1.setEndpoint("string");
        resource.setEnvironment(environment1);
        resource.setParentUuid("string");

        // run the test
        Map<ProtectedResource, List<ProtectedEnvironment>> result
            = antDBClusterInstanceConnectionChecker.collectConnectableResources(resource);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_collectConnectableResources1_should_throws_null_pointer_exception_when_objects_is_null()
        throws Exception {
        // setup
        when(protectedEnvironmentService.getEnvironmentById(anyString())).thenReturn((ProtectedEnvironment) null);

        // run the test
        Map<ProtectedResource, List<ProtectedEnvironment>> result
            = antDBClusterInstanceConnectionChecker.collectConnectableResources((ProtectedResource) null);
    }

    @Test
    public void test_applicable_should_return_true_when_condition() throws Exception {
        // setup
        ProtectedResource object = new ProtectedResource();
        object.setSubType("string");

        // run the test
        boolean result = antDBClusterInstanceConnectionChecker.applicable(object);

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_applicable_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        boolean result = antDBClusterInstanceConnectionChecker.applicable((ProtectedResource) null);
    }

    @Test
    public void test_collectConnectableResources1_should_return_not_null_when_condition2() throws Exception {
        // setup
        Map map = new HashMap<>();
        ProtectedResource protectedResource = new ProtectedResource();
        List<ProtectedEnvironment> list = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        list.add(protectedEnvironment);
        map.put(protectedResource, list);
        when(environmentRetrievalsService.collectConnectableResources(any(ProtectedResource.class))).thenReturn(map);

        ProtectedResource resource = new ProtectedResource();

        // run the test
        Map<ProtectedResource, List<ProtectedEnvironment>> result
            = antDBClusterInstanceConnectionChecker.collectConnectableResources(resource);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_collectConnectableResources1_should_throws_null_pointer_exception_when_objects_is_null1()
        throws Exception {
        // setup
        when(environmentRetrievalsService.collectConnectableResources(any(ProtectedResource.class))).thenReturn(null);

        // run the test
        Map<ProtectedResource, List<ProtectedEnvironment>> result
            = antDBClusterInstanceConnectionChecker.collectConnectableResources((ProtectedResource) null);
    }
}