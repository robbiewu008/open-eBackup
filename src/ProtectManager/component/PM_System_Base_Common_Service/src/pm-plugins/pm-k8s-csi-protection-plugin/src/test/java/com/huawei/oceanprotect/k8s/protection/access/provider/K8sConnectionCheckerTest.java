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
package com.huawei.oceanprotect.k8s.protection.access.provider;


import openbackup.access.framework.resource.service.AgentBusinessService;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import com.huawei.oceanprotect.k8s.protection.access.constant.K8sConstant;
import com.huawei.oceanprotect.k8s.protection.access.service.K8sCommonService;

import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;

/**
 * 功能描述: K8sConnectionCheckerTest
 *
 * @author t30049904
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-21
 */
public class K8sConnectionCheckerTest {
    private static  AgentBusinessService agentBusinessService = Mockito.mock(AgentBusinessService.class);
    private static ResourceService resourceService = Mockito.mock(ResourceService.class);

    private static K8sCommonService commonService;
    private static K8sConnectionChecker k8sConnectionChecker;

    @BeforeClass
    public static void init() {
        ProtectedEnvironmentRetrievalsService protectedEnvironmentRetrievalsService = Mockito.mock(
                ProtectedEnvironmentRetrievalsService.class);
        AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
        k8sConnectionChecker = new K8sConnectionChecker(protectedEnvironmentRetrievalsService,
                 agentUnifiedService, agentBusinessService, resourceService, commonService);
    }

    /**
     * 测试场景：applicable匹配成功
     * 前置条件：传入资源信息subtype类型为Kubernetes
     * 检查点：返回True
     */
    @Test
    public void test_applicable_success() {
        ProtectedResource object = new ProtectedResource();
        object.setSubType(ResourceSubTypeEnum.KUBERNETES_CLUSTER_COMMON.getType());
        Assert.assertTrue(k8sConnectionChecker.applicable(object));
    }

    /**
     * 测试场景：校验连通性报告成功
     * 前置条件：连通性检查报告中有成功的结果
     * 检查点：校验连通性报告有返回且无异常
     */
    @Test
    public void test_collect_action_results_success() {
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(Integer.valueOf(K8sConstant.SUCCESS));
        actionResult.setMessage("SUCCESS");

        CheckResult<Object> checkResult = new CheckResult<>();
        checkResult.setResults(actionResult);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("c91a4a8e-4134-3b2d-8ffe-54f589b60b17");
        checkResult.setEnvironment(environment);

        CheckReport<Object> checkReport = new CheckReport<>();
        checkReport.setResults(Collections.singletonList(checkResult));
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("c91a4a8e-4134-3b2d-8ffe-54f589b60b17");
        checkReport.setResource(resource);

        List<CheckReport<Object>> checkReports = new ArrayList<>();
        checkReports.add(checkReport);
        List<ActionResult> resultList = k8sConnectionChecker.collectActionResults(checkReports, new HashMap<>());
        Assert.assertEquals(1, resultList.size());
    }

    /**
     * 测试场景：校验连通性报告失败
     * 前置条件：连通性检查报告中只有失败的结果
     * 检查点：返回全部失败结果
     */
    @Test
    public void should_return_error_results_if_connect_failed_when_collect_action_results() {
        List<CheckReport<Object>> checkReports = new ArrayList<>();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
        actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
        actionResult.setMessage("CLUSTER_OPERATE_ERROR");

        CheckResult<Object> checkResult = new CheckResult<>();
        checkResult.setResults(actionResult);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("c91a4a8e-4134-3b2d-8ffe-54f589b60b17");
        checkResult.setEnvironment(environment);

        CheckReport<Object> checkReport = new CheckReport<>();
        checkReport.setResults(Collections.singletonList(checkResult));
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("c91a4a8e-4134-3b2d-8ffe-54f589b60b17");
        checkReport.setResource(resource);
        checkReports.add(checkReport);

        List<ActionResult> actionResults = k8sConnectionChecker.collectActionResults(checkReports, new HashMap<>());
        actionResults.forEach(actionresult -> Assert.assertTrue(
                actionresult.getCode() != ActionResult.SUCCESS_CODE));
    }

    /**
     * 用例名称：验证带环境信息的资源创建
     * 前置条件：环境信息在数据库中创建成功
     * 检查点：返回成功结果
     */
    @Test
    public void test_collect_connectable_resources_success(){
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setUuid("agent1");
        List<ProtectedEnvironment> agents = new ArrayList<>(Collections.singletonList(agent1));
        Mockito.when(agentBusinessService.queryInternalAgentEnv()).thenReturn(agents);

        ProtectedResource resource= new ProtectedResource();
        HashMap<ProtectedResource, List<ProtectedEnvironment>> resourceMap = new HashMap<>();
        resourceMap.put(resource, agents);

        Assert.assertEquals(resourceMap, k8sConnectionChecker.collectConnectableResources(resource));
    }
}