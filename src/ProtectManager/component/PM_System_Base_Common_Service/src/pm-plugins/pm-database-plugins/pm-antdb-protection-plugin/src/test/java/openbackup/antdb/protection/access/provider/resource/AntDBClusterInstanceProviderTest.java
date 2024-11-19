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
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import com.google.common.collect.ImmutableMap;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
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
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {AntDBClusterInstanceProvider.class})
@PowerMockIgnore( {"javax.management.*", "javax.net.ssl.*", "jdk.internal.reflect.*"})
public class AntDBClusterInstanceProviderTest {
    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private InstanceResourceService instanceResourceService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ResourceService resourceService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ProtectedEnvironmentService protectedEnvironmentService;

    @InjectMocks
    private AntDBClusterInstanceProvider antDBClusterInstanceProvider;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        instanceResourceService = mock(InstanceResourceService.class, Answers.RETURNS_DEEP_STUBS);
        ReflectionTestUtils.setField(antDBClusterInstanceProvider, "instanceResourceService", instanceResourceService);
        resourceService = mock(ResourceService.class, Answers.RETURNS_DEEP_STUBS);
        protectedEnvironmentService = mock(ProtectedEnvironmentService.class, Answers.RETURNS_DEEP_STUBS);
        ReflectionTestUtils.setField(antDBClusterInstanceProvider, "resourceService", resourceService);
    }

    @After
    public void tearDown() throws Exception {
    }

    @Test
    public void test_supplyDependency1_should_return_true_when_condition() throws Exception {
        // setup
        List<ProtectedResource> children = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("string");
        children.add(protectedResource);
        when(resourceService.queryDependencyResources(anyBoolean(), eq(DatabaseConstants.CHILDREN),
            anyList())).thenReturn(children);
        List<ProtectedResource> agents = new ArrayList<>();
        ProtectedResource protectedResource1 = new ProtectedResource();
        protectedResource1.setUuid("string");
        agents.add(protectedResource1);
        when(
            resourceService.queryDependencyResources(anyBoolean(), eq(DatabaseConstants.AGENTS), anyList())).thenReturn(
            agents);

        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("string");

        // run the test
        boolean result = antDBClusterInstanceProvider.supplyDependency(resource);

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_supplyDependency1_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        when(resourceService.queryDependencyResources(anyBoolean(), eq(DatabaseConstants.CHILDREN),
            anyList())).thenReturn((List) null);
        when(
            resourceService.queryDependencyResources(anyBoolean(), eq(DatabaseConstants.AGENTS), anyList())).thenReturn(
            (List) null);

        // run the test
        boolean result = antDBClusterInstanceProvider.supplyDependency((ProtectedResource) null);
    }

    @Test
    public void test_check1_should_void_when_condition() throws Exception {
        AgentBaseDto checkResult = new AgentBaseDto();
        checkResult.setErrorCode(String.valueOf(DatabaseConstants.SUCCESS_CODE));
        checkResult.setErrorMessage("{'version':'6.3.0'}");
        when(instanceResourceService.checkClusterInstance(any(ProtectedResource.class))).thenReturn(checkResult);
        AppEnvResponse clusterInstanceInfo = new AppEnvResponse();
        List<NodeInfo> nodes = new ArrayList<>();
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setEndpoint("8.40.164.14");
        nodeInfo.setExtendInfo(ImmutableMap.of(DatabaseConstants.STATUS, "1", DatabaseConstants.ROLE, "1"));
        nodes.add(nodeInfo);
        clusterInstanceInfo.setNodes(nodes);
        when(instanceResourceService.queryClusterInstanceNodeRoleByAgent(any(ProtectedResource.class))).thenReturn(
            clusterInstanceInfo);

        ProtectedResource resource = new ProtectedResource();
        Map extendInfo = new HashMap<>();
        extendInfo.put("string", "string");
        resource.setExtendInfo(extendInfo);
        resource.setUuid("string");
        resource.setName("string");
        ProtectedResource child = new ProtectedResource();
        child.setUuid("uuid");
        Map childExtendInfo = new HashMap<>();
        childExtendInfo.put(DatabaseConstants.SERVICE_IP,"8.40.164.14");
        child.setExtendInfo(childExtendInfo);
        ProtectedResource agent = new ProtectedResource();
        agent.setUuid("uuid");
        child.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, Arrays.asList(agent)));
        resource.setDependencies(ImmutableMap.of(DatabaseConstants.CHILDREN, Arrays.asList(child)));
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("string");
        when(protectedEnvironmentService.getEnvironmentById(anyString())).thenReturn(environment);
        // run the test
        antDBClusterInstanceProvider.check(resource);

        // verify the results
        assertNotNull(resource.getEnvironment());
    }

    @Test(expected = NullPointerException.class)
    public void test_check1_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        when(instanceResourceService.checkClusterInstance(any(ProtectedResource.class))).thenReturn(
            (AgentBaseDto) null);
        when(instanceResourceService.queryClusterInstanceNodeRoleByAgent(any(ProtectedResource.class))).thenReturn(
            (AppEnvResponse) null);

        // run the test
        antDBClusterInstanceProvider.check((ProtectedResource) null);
    }

    @Test
    public void test_beforeCreate_should_void_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();
        resource.setName("string");

        // run the test
        antDBClusterInstanceProvider.beforeCreate(resource);

        // verify the results
        assertEquals("string", resource.getPath());
    }

    @Test(expected = NullPointerException.class)
    public void test_beforeCreate_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBClusterInstanceProvider.beforeCreate(null);
    }

    @Test
    public void test_beforeUpdate_should_void_when_condition() throws Exception {
        // setup
        AgentBaseDto checkResult = new AgentBaseDto();
        checkResult.setErrorCode("string");
        checkResult.setErrorMessage("string");
        when(instanceResourceService.checkClusterInstance(any(ProtectedResource.class))).thenReturn(checkResult);
        AppEnvResponse clusterInstanceInfo = new AppEnvResponse();
        List<NodeInfo> nodes = new ArrayList<>();
        NodeInfo nodeInfo = new NodeInfo();
        nodes.add(nodeInfo);
        clusterInstanceInfo.setNodes(nodes);
        when(instanceResourceService.queryClusterInstanceNodeRoleByAgent(any(ProtectedResource.class))).thenReturn(
            clusterInstanceInfo);

        ProtectedResource resource = new ProtectedResource();
        Map extendInfo = new HashMap<>();
        extendInfo.put("string", "string");
        resource.setExtendInfo(extendInfo);
        resource.setUuid("string");
        resource.setName("string");

        // run the test
        antDBClusterInstanceProvider.beforeUpdate(resource);

        // verify the results
        assertNotNull(resource.getEnvironment());
    }

    @Test(expected = NullPointerException.class)
    public void test_beforeUpdate_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        // setup
        when(instanceResourceService.checkClusterInstance(any(ProtectedResource.class))).thenReturn(null);
        when(instanceResourceService.queryClusterInstanceNodeRoleByAgent(any(ProtectedResource.class))).thenReturn(
            null);

        // run the test
        antDBClusterInstanceProvider.beforeUpdate(null);
    }

    @Test
    public void test_applicable_should_return_true_when_condition() throws Exception {
        // setup
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType("string");

        // run the test
        boolean result = antDBClusterInstanceProvider.applicable(protectedResource);

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_applicable_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        boolean result = antDBClusterInstanceProvider.applicable(null);
    }

    @Test
    public void test_check1_should_void_when_condition1() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        antDBClusterInstanceProvider.check(resource);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_check1_should_throws_null_pointer_exception_when_objects_is_null1() throws Exception {

        // run the test
        antDBClusterInstanceProvider.check((ProtectedResource) null);
    }

    @Test
    public void test_updateCheck_should_void_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        antDBClusterInstanceProvider.updateCheck(resource);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_updateCheck_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBClusterInstanceProvider.updateCheck(null);
    }

    @Test
    public void test_scan_should_return_not_null_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        List<ProtectedResource> result = antDBClusterInstanceProvider.scan(resource);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_scan_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        List<ProtectedResource> result = antDBClusterInstanceProvider.scan(null);
    }

    @Test
    public void test_afterScanHandleResource_should_return_not_null_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        List<ProtectedResource> protectedResourceList = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResourceList.add(protectedResource);

        // run the test
        ExecuteScanRes result = antDBClusterInstanceProvider.afterScanHandleResource(resource, protectedResourceList);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_afterScanHandleResource_should_throws_null_pointer_exception_when_objects_is_null()
        throws Exception {

        // run the test
        ExecuteScanRes result = antDBClusterInstanceProvider.afterScanHandleResource(null, null);
    }

    @Test
    public void test_isSupportIndex_should_return_true_when_condition() throws Exception {

        // run the test
        boolean result = antDBClusterInstanceProvider.isSupportIndex();

        // verify the results
        assertTrue(result);
    }

    @Test
    public void test_getResourceFeature_should_return_not_null_when_condition() throws Exception {

        // run the test
        ResourceFeature result = antDBClusterInstanceProvider.getResourceFeature();

        // verify the results
        assertNotNull(result);
    }

    @Test
    public void test_preHandleDelete_should_return_not_null_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        ResourceDeleteContext result = antDBClusterInstanceProvider.preHandleDelete(resource);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_preHandleDelete_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        ResourceDeleteContext result = antDBClusterInstanceProvider.preHandleDelete(null);
    }

    @Test
    public void test_healthCheck_should_void_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        antDBClusterInstanceProvider.healthCheck(resource);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_healthCheck_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBClusterInstanceProvider.healthCheck(null);
    }

    @Test
    public void test_cleanUnmodifiableFieldsWhenUpdate_should_void_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        antDBClusterInstanceProvider.cleanUnmodifiableFieldsWhenUpdate(resource);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_cleanUnmodifiableFieldsWhenUpdate_should_throws_null_pointer_exception_when_objects_is_null()
        throws Exception {

        // run the test
        antDBClusterInstanceProvider.cleanUnmodifiableFieldsWhenUpdate(null);
    }

    @Test
    public void test_queryRelationResourceToDelete_should_return_not_null_when_condition() throws Exception {
        // setup
        ProtectedResource protectedResource = new ProtectedResource();

        // run the test
        Set<String> result = antDBClusterInstanceProvider.queryRelationResourceToDelete(protectedResource);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_queryRelationResourceToDelete_should_throws_null_pointer_exception_when_objects_is_null()
        throws Exception {

        // run the test
        Set<String> result = antDBClusterInstanceProvider.queryRelationResourceToDelete(null);
    }

    @Test
    public void test_supplyDependency1_should_return_true_when_condition1() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();

        // run the test
        boolean result = antDBClusterInstanceProvider.supplyDependency(resource);

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_supplyDependency1_should_throws_null_pointer_exception_when_objects_is_null1() throws Exception {

        // run the test
        boolean result = antDBClusterInstanceProvider.supplyDependency((ProtectedResource) null);
    }

    @Test
    public void test_queryAppConf_should_return_not_null_when_condition() throws Exception {
        // setup
        String[] hostUuids = new String[0];

        // run the test
        Map<String, Object> result = antDBClusterInstanceProvider.queryAppConf("string", hostUuids);

        // verify the results
        assertNotNull(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_queryAppConf_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        Map<String, Object> result = antDBClusterInstanceProvider.queryAppConf(null, null);
    }
}