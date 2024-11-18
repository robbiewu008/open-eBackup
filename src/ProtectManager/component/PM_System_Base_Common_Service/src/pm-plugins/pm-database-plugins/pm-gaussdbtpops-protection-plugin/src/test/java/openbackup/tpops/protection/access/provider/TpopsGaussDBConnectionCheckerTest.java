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
package openbackup.tpops.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * The GaussDBConnectionCheckerTest
 *
 */
@RunWith(PowerMockRunner.class)
public class TpopsGaussDBConnectionCheckerTest {
    private TpopsGaussDBConnectionChecker tpopsGaussDBConnectionChecker;

    private AgentUnifiedService agentUnifiedService;

    private final ProtectedEnvironmentService protectedEnvironmentService = PowerMockito.mock(
        ProtectedEnvironmentService.class);

    @Before
    public void init() throws IllegalAccessException {
        ProtectedEnvironmentRetrievalsService environmentRetrievalsService = PowerMockito.mock(
            ProtectedEnvironmentRetrievalsService.class);
        agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);
        tpopsGaussDBConnectionChecker = new TpopsGaussDBConnectionChecker(environmentRetrievalsService,
            agentUnifiedService);
        MemberModifier.field(TpopsGaussDBConnectionChecker.class, "protectedEnvironmentService")
            .set(tpopsGaussDBConnectionChecker, protectedEnvironmentService);
    }

    /**
     * 用例场景：GaussDB 集群环境 联通性provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT.getType());
        Assert.assertTrue(tpopsGaussDBConnectionChecker.applicable(protectedResource));
    }

    /**
     * 用例场景： check结果检查
     * 前置条件：无
     * 检查点：返回成功
     */
    @Test
    public void generate_check_result() {
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> stringListMap = Optional.ofNullable(protectedResource.getDependencies())
            .orElse(new HashMap<>());
        List<ProtectedResource> protectedResources = new ArrayList<>();
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("123456");
        protectedResources.add(resource);
        protectedResources.add(resource);
        stringListMap.put(TpopsGaussDBConstant.GAUSSDB_AGENTS, protectedResources);
        protectedResource.setDependencies(stringListMap);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("123456");
        protectedResource.setEnvironment(protectedEnvironment);
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplicationNoRetry(any(), any())).thenReturn(agentBaseDto);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById("123456"))
            .thenReturn(getProtectedEnvironment());
        tpopsGaussDBConnectionChecker.generateCheckResult(protectedResource);
    }

    private static ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("aaaaaaaaaaaaaaaaa");
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(TpopsGaussDBConstant.GAUSSDB_AGENTS, getProtectedResources("bbbbbbbbbbbbb"));
        resource.setDependencies(dependencies);
        Authentication authentication = new Authentication();
        authentication.setAuthKey("omm");
        resource.setAuth(authentication);
        resource.setExtendInfo(new HashMap<>());
        resource.getExtendInfo().put(TpopsGaussDBConstant.EXTEND_INFO_KEY_PROJECT_NAME, "PROJECT_NAME");
        resource.getExtendInfo().put(TpopsGaussDBConstant.EXTEND_INFO_KEY_ACCOUNT_NAME, "ACCOUNT_NAME");
        resource.getExtendInfo().put(TpopsGaussDBConstant.EXTEND_INFO_KEY_PM_ADDRESS, "PM_ADDRESS");
        return resource;
    }

    private static List<ProtectedResource> getProtectedResources(String uuid) {
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(uuid);
        list.add(protectedResource);
        return list;
    }

    /**
     * 用例场景：collect结果检查
     * 前置条件：无
     * 检查点：设置正确，返回成功
     */
    @Test
    public void collect_action_results() {
        List<CheckReport<Object>> checkReport = new ArrayList<>();
        CheckReport<Object> objectCheckReport = new CheckReport<>();
        List<CheckResult<Object>> list = new ArrayList<>();
        CheckResult<Object> checkResult_1 = new CheckResult<>();
        ActionResult actionResult_1 = new ActionResult();
        actionResult_1.setCode(200);
        checkResult_1.setResults(actionResult_1);
        list.add(checkResult_1);
        CheckResult<Object> checkResult_2 = new CheckResult<>();
        ActionResult actionResult_2 = new ActionResult();
        actionResult_2.setCode(200);
        actionResult_2.setBodyErr("19952785");
        checkResult_2.setResults(actionResult_2);
        list.add(checkResult_2);
        objectCheckReport.setResults(list);
        checkReport.add(objectCheckReport);
        Assert.assertEquals(2, tpopsGaussDBConnectionChecker.collectActionResults(checkReport, new HashMap<>()).size());
    }
}