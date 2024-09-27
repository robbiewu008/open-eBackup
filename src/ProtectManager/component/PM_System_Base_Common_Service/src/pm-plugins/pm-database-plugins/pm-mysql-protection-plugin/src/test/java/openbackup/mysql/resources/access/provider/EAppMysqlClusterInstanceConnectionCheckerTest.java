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
package openbackup.mysql.resources.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.List;
import java.util.Map;

/**
 * eapp实例检查LLT
 *
 * @author fWX1071802
 * @since 2023/8/5
 */
@RunWith(PowerMockRunner.class)
public class EAppMysqlClusterInstanceConnectionCheckerTest {

    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private final MysqlBaseService mysqlBaseService = Mockito.mock(MysqlBaseService.class);

    ProtectedEnvironmentRetrievalsService environmentRetrievalsService = Mockito.mock(
        ProtectedEnvironmentRetrievalsService.class);

    private final ProtectedEnvironmentService protectedEnvironmentService = Mockito.mock(
        ProtectedEnvironmentService.class);

    private EAppMysqlClusterInstanceConnectionChecker instance;

    @Rule
    ExpectedException exception = ExpectedException.none();

    @Before
    public void init() {
        instance = new EAppMysqlClusterInstanceConnectionChecker(environmentRetrievalsService, agentUnifiedService,
            mysqlBaseService, protectedEnvironmentService);
    }

    @Test
    public void check_generateCheckResult_when_response_empty() {
        ProtectedResource node = new ProtectedResource();
        node.setAuth(new Authentication());
        node.setUuid("uuid");
        PowerMockito.when(mysqlBaseService.getResource("uuid")).thenReturn(node);
        node.setExtendInfo(ImmutableMap.of(DatabaseConstants.HOST_ID, "hostid"));
        ProtectedEnvironment env = new ProtectedEnvironment();
        PowerMockito.when(mysqlBaseService.getEnvironmentById("hostid")).thenReturn(env);
        PowerMockito.when(agentUnifiedService.getClusterInfo(node, env)).thenReturn(null);
        CheckResult<Object> result = instance.generateCheckResult(node);
        Assert.assertEquals(result.getResults().getCode(), CommonErrorCode.AGENT_NETWORK_ERROR);
    }

    @Test
    public void check_generateCheckResult_when_response_error() {
        ProtectedResource node = new ProtectedResource();
        node.setAuth(new Authentication());
        node.setUuid("uuid");
        PowerMockito.when(mysqlBaseService.getResource("uuid")).thenReturn(node);
        node.setExtendInfo(ImmutableMap.of(DatabaseConstants.HOST_ID, "hostid"));
        ProtectedEnvironment env = new ProtectedEnvironment();
        PowerMockito.when(mysqlBaseService.getEnvironmentById("hostid")).thenReturn(env);
        AppEnvResponse response = new AppEnvResponse();
        response.setExtendInfo(ImmutableMap.of(MysqlConstants.ERROR_CODE, "1"));
        PowerMockito.when(agentUnifiedService.getClusterInfo(node, env)).thenReturn(response);
        CheckResult<Object> result = instance.generateCheckResult(node);
        Assert.assertEquals(result.getResults().getCode(), 1);
    }

    @Test
    public void test_collectConnectableResources() {
        ProtectedResource resource = new ProtectedResource();
        ProtectedResource node = new ProtectedResource();
        resource.setDependencies(ImmutableMap.of(DatabaseConstants.CHILDREN, Lists.newArrayList(node)));
        node.setExtendInfo(ImmutableMap.of(DatabaseConstants.HOST_ID, "hostid"));
        ProtectedEnvironment env = new ProtectedEnvironment();
        PowerMockito.when(mysqlBaseService.getEnvironmentById("1")).thenReturn(env);
        Map<ProtectedResource, List<ProtectedEnvironment>> map = instance.collectConnectableResources(resource);
        Assert.assertNotNull(map.get(node));
    }

    @Test
    @Ignore
    public void test_applicable_when_eapp(){
        ProtectedResource resource = new ProtectedResource();
        resource.setExtendInfo(ImmutableMap.of(DatabaseConstants.CLUSTER_TYPE, MysqlConstants.EAPP));
        Assert.assertTrue(instance.applicable(resource));
    }

    @Test
    public void test_applicable_when_empty(){
        Assert.assertFalse(instance.applicable(null));
    }
}
