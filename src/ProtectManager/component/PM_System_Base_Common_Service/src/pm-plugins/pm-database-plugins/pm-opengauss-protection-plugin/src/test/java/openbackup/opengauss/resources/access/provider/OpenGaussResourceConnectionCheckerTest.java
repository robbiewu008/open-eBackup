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
package openbackup.opengauss.resources.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.opengauss.resources.access.constants.OpenGaussConstants;
import openbackup.opengauss.resources.access.service.OpenGaussAgentService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.ArgumentMatchers;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * OpenGauss资源检查checker测试类
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-21
 */
public class OpenGaussResourceConnectionCheckerTest {
    private final ProtectedEnvironmentRetrievalsService environmentRetrievalsService = PowerMockito.mock(ProtectedEnvironmentRetrievalsService.class);
    private final AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);
    private final OpenGaussAgentService openGaussAgentService = PowerMockito.mock(OpenGaussAgentService.class);
    private final OpenGaussResourceConnectionChecker openGaussResourceConnectionChecker
        = new OpenGaussResourceConnectionChecker(environmentRetrievalsService, agentUnifiedService, openGaussAgentService);

    /**
     * 用例场景：OpenGauss资源检查checker
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.OPENGAUSS.getType());
        Assert.assertTrue(openGaussResourceConnectionChecker.applicable(protectedResource));
    }

    /**
     * 用例场景：获取检查结果
     * 前置条件：连通性检查成功
     * 检查点： 集群节点返回的系统id为空或者不唯一，检测失败
     */
    @Test
    public void collect_action_results_failure() {
        List<CheckReport<Object>> checkReports = getCheckReports(getSuccessActionResult());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> openGaussResourceConnectionChecker.collectActionResults(checkReports, new HashMap<>()));
        Assert.assertEquals("The openGauss selected cluster type does not match the application cluster type.",
            legoCheckedException.getMessage());
    }

    /**
     * 构建检查报告 成功或者失败
     *
     * @param actionResult 报告结果
     * @return 检查报告
     */
    private List<CheckReport<Object>> getCheckReports(ActionResult actionResult) {
        List<CheckReport<Object>> checkReports = new ArrayList<>();
        CheckReport<Object> checkReport = new CheckReport<>();
        List<CheckResult<Object>> results = new ArrayList<>();
        CheckResult<Object> result = new CheckResult<>();
        results.add(result);
        result.setResults(actionResult);
        checkReport.setResults(results);
        checkReports.add(checkReport);
        return checkReports;
    }

    private ActionResult getSuccessActionResult() {
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(0);
        actionResult.setMessage("success");
        return actionResult;
    }

    /**
     * 用例场景：获取检查结果
     * 前置条件：连通性检查成功
     * 检查点： 连通性检查成功
     */
    @Test
    public void should_throw_LegoCheckedException_if_generate_check_result_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("123");
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplication(ArgumentMatchers.any(), ArgumentMatchers.any()))
            .thenReturn(agentBaseDto);
        CheckResult<Object> result = openGaussResourceConnectionChecker.generateCheckResult(protectedResource);
        Assert.assertEquals(0L, result.getResults().getCode());
    }

    /**
     * 用例场景：检查节点是否是同一个集群
     * 前置条件：OpenGauss的节点systemId不一致
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_system_id_is_not_unique() {
        CheckResult<Object> checkResult = getCheckResult("", "", "");
        CheckResult<Object> checkResult2 = getCheckResult("10100041316029899601", "", "");
        CheckResult<Object> checkResult3 = getCheckResult("10100041316029899602", "", "");
        List<CheckReport<Object>> checkReports = new ArrayList<>();
        CheckReport<Object> checkReport = new CheckReport<>();
        List<CheckResult<Object>> checkResults = new ArrayList<>(
            Arrays.asList(checkResult, checkResult2, checkResult3));
        checkReport.setResults(checkResults);
        checkReports.add(checkReport);
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(openGaussResourceConnectionChecker, "checkClusterUnique", checkReports));
    }

    /**
     * 用例场景：检查节点是否是同一个集群
     * 前置条件：OpenGauss的节点一致但是节点的ip不一致
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_nodes_is_not_in_a_same_cluster() {
        CheckResult<Object> checkResult = getCheckResult("10100041316029899601", "127.0.0.1", "127.0.0.2");
        CheckResult<Object> checkResult2 = getCheckResult("10100041316029899601", "127.0.0.3", "127.0.0.4");
        List<CheckReport<Object>> checkReports = new ArrayList<>();
        CheckReport<Object> checkReport = new CheckReport<>();
        List<CheckResult<Object>> checkResults = new ArrayList<>(
            Arrays.asList(checkResult, checkResult2));
        checkReport.setResults(checkResults);
        checkReports.add(checkReport);
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(openGaussResourceConnectionChecker, "checkClusterUnique", checkReports));
    }

    private CheckResult<Object> getCheckResult(String systemId, String endPoint1, String endPoint2) {
        AppEnvResponse appEnvResponse1 = new AppEnvResponse();
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setEndpoint(endPoint1);
        NodeInfo nodeInfo2 = new NodeInfo();
        nodeInfo2.setEndpoint(endPoint2);
        appEnvResponse1.setNodes(Arrays.asList(nodeInfo, nodeInfo2));
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(OpenGaussConstants.SYSTEM_ID, systemId);
        appEnvResponse1.setExtendInfo(extendInfo);
        CheckResult<Object> checkResult = new CheckResult<>();
        checkResult.setData(appEnvResponse1);
        return checkResult;
    }
}