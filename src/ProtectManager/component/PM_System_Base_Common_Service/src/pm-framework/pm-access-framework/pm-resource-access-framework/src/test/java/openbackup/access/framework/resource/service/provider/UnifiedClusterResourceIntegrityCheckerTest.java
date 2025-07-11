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
package openbackup.access.framework.resource.service.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.List;

@RunWith(PowerMockRunner.class)
public class UnifiedClusterResourceIntegrityCheckerTest {
    private AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private ProtectedEnvironmentRetrievalsService environmentRetrievalsService = Mockito.mock(
        ProtectedEnvironmentRetrievalsService.class);

    private UnifiedClusterResourceIntegrityChecker checker = new UnifiedClusterResourceIntegrityChecker(
        environmentRetrievalsService, agentUnifiedService);

    /**
     * 用例场景：校验检查结果是否成功返回
     * 前置条件：环境正常
     * 检查点：正常返回检查出来的环境和集群信息
     */
    @Test
    public void generate_check_result_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setName("test");
        resource.setType("OceanV6");
        resource.setSubType("CloudBackupFilesystem");
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("test");
        environment.setUsername("testadmin");
        environment.setPassword("123456");
        resource.setEnvironment(environment);
        PowerMockito.when(agentUnifiedService.getClusterInfo(PowerMockito.mock(ProtectedResource.class),
            PowerMockito.mock(ProtectedEnvironment.class))).thenReturn(new AppEnvResponse());
       Assert.assertNotNull(checker.generateCheckResult(resource));
    }

    /**
     * 用例场景：校验集群完整性
     * 前置条件：环境正常
     * 检查点：正常返回检查结果
     */
    @Test
    public void check_cluster_integrity_success() throws Exception {
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setName("test");
        nodeInfo.setEndpoint("127.0.0.2");
        List<NodeInfo> nodes = new ArrayList<>();
        nodes.add(nodeInfo);
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setName("test");
        appEnvResponse.setEndpoint("127.0.0.2");
        appEnvResponse.setNodes(nodes);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("test");
        environment.setEndpoint("127.0.0.2");
        environment.setUsername("admin");
        environment.setPassword("admin123");
        CheckResult<AppEnvResponse> checkResult = new CheckResult<>();
        checkResult.setData(appEnvResponse);
        checkResult.setEnvironment(environment);
        List<CheckResult<AppEnvResponse>> results = new ArrayList<>();
        results.add(checkResult);
        CheckReport<AppEnvResponse> checkReport = new CheckReport<>();
        checkReport.setResults(results);
        Assert.assertNotNull( Whitebox.invokeMethod(checker,"checkClusterIntegrity",checkReport));
    }
}
