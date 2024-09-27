/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.ndmp.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.ndmp.protection.access.constant.NdmpConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 联通性检查
 *
 * @author t30021437
 * @since 2023-05-11
 */
public class NdmpConnectionCheckerTest {
    private NdmpConnectionChecker ndmpConnectionChecker;

    private AgentUnifiedService agentUnifiedService;

    private final ProtectedEnvironmentService protectedEnvironmentService = PowerMockito.mock(
        ProtectedEnvironmentService.class);

    @Before
    public void init() throws IllegalAccessException {
        ProtectedEnvironmentRetrievalsService environmentRetrievalsService = PowerMockito.mock(
            ProtectedEnvironmentRetrievalsService.class);
        agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);
        ndmpConnectionChecker = new NdmpConnectionChecker(environmentRetrievalsService, agentUnifiedService,
            protectedEnvironmentService);
        MemberModifier.field(NdmpConnectionChecker.class, "protectedEnvironmentService")
            .set(ndmpConnectionChecker, protectedEnvironmentService);
    }

    /**
     * 用例场景：ndmp 集群环境 联通性provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.NDMP.getType());
        Assert.assertTrue(ndmpConnectionChecker.applicable(protectedResource));
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
        stringListMap.put(NdmpConstant.AGENTS, protectedResources);
        protectedResource.setDependencies(stringListMap);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("123456");
        protectedResource.setEnvironment(protectedEnvironment);
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplicationNoRetry(any(), any())).thenReturn(agentBaseDto);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById("123456"))
            .thenReturn(getProtectedEnvironment());
        ndmpConnectionChecker.generateCheckResult(protectedResource);
    }

    private static ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("aaaaaaaaaaaaaaaaa");
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(NdmpConstant.AGENTS, getProtectedResources("bbbbbbbbbbbbb"));
        resource.setDependencies(dependencies);
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
    public void collectActionResults() {
        CheckResult<Object> checkResultOne = new CheckResult<>();
        ActionResult actionResultOne = new ActionResult();
        actionResultOne.setCode(200);
        checkResultOne.setResults(actionResultOne);
        List<CheckResult<Object>> list = new ArrayList<>();
        list.add(checkResultOne);
        CheckResult<Object> checkResultTwo = new CheckResult<>();
        ActionResult actionResultTwo = new ActionResult();
        actionResultTwo.setCode(200);
        actionResultTwo.setBodyErr("999");
        checkResultTwo.setResults(actionResultTwo);
        list.add(checkResultTwo);
        CheckReport<Object> objectCheckReport = new CheckReport<>();
        objectCheckReport.setResults(list);
        List<CheckReport<Object>> checkReport = new ArrayList<>();
        checkReport.add(objectCheckReport);
        Assert.assertEquals(2, ndmpConnectionChecker.collectActionResults(checkReport, new HashMap<>()).size());
    }
}