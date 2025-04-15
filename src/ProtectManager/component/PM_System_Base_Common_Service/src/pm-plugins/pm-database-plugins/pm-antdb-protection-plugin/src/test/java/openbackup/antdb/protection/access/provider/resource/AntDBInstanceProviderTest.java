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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyList;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.model.ExecuteScanRes;
import openbackup.database.base.plugin.common.DatabaseConstants;
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
import java.util.List;
import java.util.Map;
import java.util.Set;

@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {AntDBInstanceProvider.class})
@PowerMockIgnore( {"javax.management.*", "javax.net.ssl.*", "jdk.internal.reflect.*"})
public class AntDBInstanceProviderTest {
    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ProviderManager providerManager;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private InstanceResourceService instanceResourceService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ResourceService resourceService;

    @InjectMocks
    private AntDBInstanceProvider antDBInstanceProvider;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        providerManager = mock(ProviderManager.class, Answers.RETURNS_DEEP_STUBS);
        ReflectionTestUtils.setField(antDBInstanceProvider, "providerManager", providerManager);
        instanceResourceService = mock(InstanceResourceService.class, Answers.RETURNS_DEEP_STUBS);
        ReflectionTestUtils.setField(antDBInstanceProvider, "instanceResourceService", instanceResourceService);
        resourceService = mock(ResourceService.class, Answers.RETURNS_DEEP_STUBS);
        ReflectionTestUtils.setField(antDBInstanceProvider, "resourceService", resourceService);
    }

    @After
    public void tearDown() throws Exception {
    }

    @Test
    public void test_supplyDependency1_should_return_true_when_condition() throws Exception {
        // setup
        List<ProtectedResource> agents = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("string");
        agents.add(protectedResource);
        when(
            resourceService.queryDependencyResources(anyBoolean(), eq(DatabaseConstants.AGENTS), anyList())).thenReturn(
            agents);

        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("string");

        // run the test
        boolean result = antDBInstanceProvider.supplyDependency(resource);

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_supplyDependency1_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        when(
            resourceService.queryDependencyResources(anyBoolean(), eq(DatabaseConstants.AGENTS), anyList())).thenReturn(
            (List) null);

        // run the test
        boolean result = antDBInstanceProvider.supplyDependency((ProtectedResource) null);
    }

    @Test
    public void test_beforeCreate_should_void_when_condition() throws Exception {
        // setup
        ResourceConnectionCheckProvider provider = mock(ResourceConnectionCheckProvider.class);
        ResourceCheckContext context = new ResourceCheckContext();
        List<ActionResult> actionResults = new ArrayList<>();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(0L);
        actionResult.setBodyErr("string");
        actionResult.setMessage("string");
        actionResults.add(actionResult);
        context.setActionResults(actionResults);
        when(provider.tryCheckConnection(any(ProtectedResource.class))).thenReturn(context);
        when(providerManager.findProvider(eq(ResourceConnectionCheckProvider.class),
            any(ProtectedResource.class))).thenReturn(provider);

        ProtectedResource resource = new ProtectedResource();
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("string");
        resource.setEnvironment(environment);
        resource.setName("string");

        // run the test
        antDBInstanceProvider.beforeCreate(resource);

        // verify the results
        assertEquals("string", resource.getPath());
    }

    @Test(expected = NullPointerException.class)
    public void test_beforeCreate_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        ResourceConnectionCheckProvider provider = mock(ResourceConnectionCheckProvider.class);
        when(provider.tryCheckConnection(any(ProtectedResource.class))).thenReturn(null);
        when(providerManager.findProvider(eq(ResourceConnectionCheckProvider.class),
            any(ProtectedResource.class))).thenReturn(provider);

        // run the test
        antDBInstanceProvider.beforeCreate(null);
    }

    @Test
    public void test_beforeUpdate_should_void_when_condition() throws Exception {
        // setup
        ResourceConnectionCheckProvider provider = mock(ResourceConnectionCheckProvider.class);
        ResourceCheckContext context = new ResourceCheckContext();
        List<ActionResult> actionResults = new ArrayList<>();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(0L);
        actionResult.setBodyErr("string");
        actionResult.setMessage("string");
        actionResults.add(actionResult);
        context.setActionResults(actionResults);
        when(provider.tryCheckConnection(any(ProtectedResource.class))).thenReturn(context);
        when(providerManager.findProvider(eq(ResourceConnectionCheckProvider.class),
            any(ProtectedResource.class))).thenReturn(provider);

        ProtectedResource resource = new ProtectedResource();
        resource.setName("string");

        // run the test
        antDBInstanceProvider.beforeUpdate(resource);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_beforeUpdate_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        ResourceConnectionCheckProvider provider = mock(ResourceConnectionCheckProvider.class);
        when(provider.tryCheckConnection(any(ProtectedResource.class))).thenReturn(null);
        when(providerManager.findProvider(eq(ResourceConnectionCheckProvider.class),
            any(ProtectedResource.class))).thenReturn(provider);

        // run the test
        antDBInstanceProvider.beforeUpdate(null);
    }

    @Test
    public void test_healthCheck1_should_void_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        antDBInstanceProvider.healthCheck(resource);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_healthCheck1_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBInstanceProvider.healthCheck((ProtectedResource) null);
    }

    @Test
    public void test_applicable_should_return_true_when_condition() throws Exception {
        // setup
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType("string");

        // run the test
        boolean result = antDBInstanceProvider.applicable(protectedResource);

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_applicable_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        boolean result = antDBInstanceProvider.applicable(null);
    }

    @Test
    public void test_check_should_void_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        antDBInstanceProvider.check(resource);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_check_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBInstanceProvider.check(null);
    }

    @Test
    public void test_updateCheck_should_void_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        antDBInstanceProvider.updateCheck(resource);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_updateCheck_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBInstanceProvider.updateCheck(null);
    }

    @Test
    public void test_scan_should_return_not_null_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        List<ProtectedResource> result = antDBInstanceProvider.scan(resource);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_scan_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        List<ProtectedResource> result = antDBInstanceProvider.scan(null);
    }

    @Test
    public void test_afterScanHandleResource_should_return_not_null_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        List<ProtectedResource> protectedResourceList = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResourceList.add(protectedResource);

        // run the test
        ExecuteScanRes result = antDBInstanceProvider.afterScanHandleResource(resource, protectedResourceList);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_afterScanHandleResource_should_throws_null_pointer_exception_when_objects_is_null()
        throws Exception {

        // run the test
        ExecuteScanRes result = antDBInstanceProvider.afterScanHandleResource(null, null);
    }

    @Test
    public void test_isSupportIndex_should_return_true_when_condition() throws Exception {

        // run the test
        boolean result = antDBInstanceProvider.isSupportIndex();

        // verify the results
        assertTrue(result);
    }

    @Test
    public void test_getResourceFeature_should_return_not_null_when_condition() throws Exception {

        // run the test
        ResourceFeature result = antDBInstanceProvider.getResourceFeature();

        // verify the results
        assertNotNull(result);
    }

    @Test
    public void test_preHandleDelete_should_return_not_null_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        ResourceDeleteContext result = antDBInstanceProvider.preHandleDelete(resource);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_preHandleDelete_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        ResourceDeleteContext result = antDBInstanceProvider.preHandleDelete(null);
    }

    @Test
    public void test_cleanUnmodifiableFieldsWhenUpdate_should_void_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        antDBInstanceProvider.cleanUnmodifiableFieldsWhenUpdate(resource);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_cleanUnmodifiableFieldsWhenUpdate_should_throws_null_pointer_exception_when_objects_is_null()
        throws Exception {

        // run the test
        antDBInstanceProvider.cleanUnmodifiableFieldsWhenUpdate(null);
    }

    @Test
    public void test_queryRelationResourceToDelete_should_return_not_null_when_condition() throws Exception {
        // setup
        ProtectedResource protectedResource = new ProtectedResource();

        // run the test
        Set<String> result = antDBInstanceProvider.queryRelationResourceToDelete(protectedResource);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_queryRelationResourceToDelete_should_throws_null_pointer_exception_when_objects_is_null()
        throws Exception {

        // run the test
        Set<String> result = antDBInstanceProvider.queryRelationResourceToDelete(null);
    }

    @Test
    public void test_supplyDependency1_should_return_true_when_condition1() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        boolean result = antDBInstanceProvider.supplyDependency(resource);

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_supplyDependency1_should_throws_null_pointer_exception_when_objects_is_null1() throws Exception {

        // run the test
        boolean result = antDBInstanceProvider.supplyDependency((ProtectedResource) null);
    }

    @Test
    public void test_queryAppConf_should_return_not_null_when_condition() throws Exception {
        // setup
        String[] hostUuids = new String[0];

        // run the test
        Map<String, Object> result = antDBInstanceProvider.queryAppConf("string", hostUuids);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_queryAppConf_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        Map<String, Object> result = antDBInstanceProvider.queryAppConf(null, null);
    }
}