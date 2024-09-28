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
package openbackup.saphana.protection.access.provider.resource;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.saphana.protection.access.constant.SapHanaConstants;
import openbackup.saphana.protection.access.service.SapHanaResourceService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.ArgumentMatchers;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * {@link SapHanaDbConnectionChecker Test}
 *
 */
public class SapHanaDbConnectionCheckerTest {
    private final ProtectedEnvironmentRetrievalsService environmentRetrievalsService = PowerMockito.mock(
        ProtectedEnvironmentRetrievalsService.class);

    private final AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final ProtectedEnvironmentService environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);

    private final SapHanaResourceService hanaResourceService = PowerMockito.mock(SapHanaResourceService.class);

    private final SapHanaDbConnectionChecker dbConnChecker = new SapHanaDbConnectionChecker(
        environmentRetrievalsService, agentUnifiedService, resourceService, environmentService, hanaResourceService);

    /**
     * 用例场景：收集连通测试需要的资源信息
     * 前置条件：输入连通测试的资源信息
     * 检查点：返回需要的资源信息
     */
    @Test
    public void collectConnectableResources_success() {
        ProtectedResource dbResource = mockClusterDbResource(false, true);
        Map<ProtectedResource, List<ProtectedEnvironment>> resourceListMap = dbConnChecker.collectConnectableResources(
            dbResource);
        Assert.assertEquals(2, resourceListMap.get(dbResource).size());
    }

    /**
     * 用例场景：收集连通测试结果列表
     * 前置条件：输入的检查报告列表为空
     * 检查点：返回空列表
     */
    @Test
    public void test_should_return_empty_list_if_db_report_list_is_empty_when_collectActionResults() {
        List<CheckReport<Object>> reportList = new ArrayList<>();
        Map<String, Object> context = new HashMap<>();
        List<ActionResult> resultList = dbConnChecker.collectActionResults(reportList, context);
        Assert.assertEquals(0, resultList.size());
    }

    /**
     * 用例场景：收集连通测试结果列表
     * 前置条件：注册场景，输入合法的检查报告
     * 检查点：返回测试结果列表
     */
    @Test
    public void test_should_return_non_empty_list_if_register_db_when_collectActionResults() {
        PowerMockito.when(resourceService.getResourceById("3717e3b69a014535866e91cb6b42949c"))
            .thenReturn(Optional.empty());
        List<CheckReport<Object>> reportList = mockClusterDbConnCheckReport(false, true, true, false);
        Map<String, Object> context = new HashMap<>();
        List<ActionResult> resultList = dbConnChecker.collectActionResults(reportList, context);
        List<ActionResult> failResultList = resultList.stream()
            .filter(result -> result.getCode() != DatabaseConstants.SUCCESS_CODE)
            .collect(Collectors.toList());
        Assert.assertEquals(2, resultList.size());
        Assert.assertEquals(1, failResultList.size());
    }

    /**
     * 用例场景：收集连通测试结果列表
     * 前置条件：修改场景，输入合法的检查报告
     * 检查点：返回测试结果列表
     */
    @Test
    public void test_should_return_non_empty_list_if_modify_db_when_collectActionResults() {
        PowerMockito.when(resourceService.getResourceById("3717e3b69a014535866e91cb6b42949c"))
            .thenReturn(Optional.of(mockClusterDbResource(false, true)));
        List<CheckReport<Object>> reportList = mockClusterDbConnCheckReport(true, true, false, false);
        ProtectedResource instResourceOfReport = reportList.get(0).getResource();
        PowerMockito.when(hanaResourceService.isModifyResource(instResourceOfReport)).thenReturn(true);
        Map<String, Object> context = new HashMap<>();
        List<ActionResult> resultList = dbConnChecker.collectActionResults(reportList, context);
        List<ActionResult> successResultList = resultList.stream()
            .filter(result -> result.getCode() == DatabaseConstants.SUCCESS_CODE)
            .collect(Collectors.toList());
        Assert.assertEquals(2, resultList.size());
        Assert.assertEquals(0, successResultList.size());
    }

    /**
     * 用例场景：收集连通测试结果列表
     * 前置条件：连通测试场景，输入合法的检查报告，系统数据库有一个节点检查成功
     * 检查点：返回测试结果列表
     */
    @Test
    public void test_should_return_non_empty_list_if_conn_system_db_online_when_collectActionResults() {
        PowerMockito.when(resourceService.getResourceById("3717e3b69a014535866e91cb6b42949c"))
            .thenReturn(Optional.of(mockClusterDbResource(false, true)));
        List<CheckReport<Object>> reportList = mockClusterDbConnCheckReport(false, true, false, true);
        ProtectedResource instResourceOfReport = reportList.get(0).getResource();
        PowerMockito.when(hanaResourceService.isModifyResource(instResourceOfReport)).thenReturn(false);
        PowerMockito.when(hanaResourceService.getResourceById("3717e3b69a014535866e91cb6b42949c"))
            .thenReturn(mockClusterDbResource(false, true));
        PowerMockito.doReturn(mockClusterInstance())
            .when(environmentService)
            .getEnvironmentById(ArgumentMatchers.anyString());
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .updateDbLinkStatus(ArgumentMatchers.any(ProtectedResource.class), ArgumentMatchers.eq("1"));
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .updateInstanceLinkStatus(ArgumentMatchers.any(ProtectedEnvironment.class), ArgumentMatchers.eq("1"));
        Map<String, Object> context = new HashMap<>();
        List<ActionResult> resultList = dbConnChecker.collectActionResults(reportList, context);
        List<ActionResult> successResultList = resultList.stream()
            .filter(result -> result.getCode() == DatabaseConstants.SUCCESS_CODE)
            .collect(Collectors.toList());
        Assert.assertEquals(2, resultList.size());
        Assert.assertEquals(1, successResultList.size());
    }

    /**
     * 用例场景：收集连通测试结果列表
     * 前置条件：连通测试场景，输入合法的检查报告，租户数据库有一个节点检查失败
     * 检查点：返回测试结果列表
     */
    @Test
    public void test_should_return_non_empty_list_if_conn_tenant_db_offline_when_collectActionResults() {
        PowerMockito.when(resourceService.getResourceById("3717e3b69a014535866e91cb6b42949c"))
            .thenReturn(Optional.of(mockClusterDbResource(false, false)));
        List<CheckReport<Object>> reportList = mockClusterDbConnCheckReport(false, false, false, true);
        ProtectedResource instResourceOfReport = reportList.get(0).getResource();
        PowerMockito.when(hanaResourceService.isModifyResource(instResourceOfReport)).thenReturn(false);
        PowerMockito.when(hanaResourceService.getResourceById("3717e3b69a014535866e91cb6b42949c"))
            .thenReturn(mockClusterDbResource(false, false));
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .updateDbLinkStatus(ArgumentMatchers.any(ProtectedResource.class), ArgumentMatchers.eq("0"));
        Map<String, Object> context = new HashMap<>();
        List<ActionResult> resultList = dbConnChecker.collectActionResults(reportList, context);
        List<ActionResult> failResultList = resultList.stream()
            .filter(result -> result.getCode() != DatabaseConstants.SUCCESS_CODE)
            .collect(Collectors.toList());
        Assert.assertEquals(2, resultList.size());
        Assert.assertEquals(1, failResultList.size());
    }

    /**
     * 用例场景：框架调applicable接口
     * 前置条件：applicable输入资源信息
     * 检查点：SAPHANA-database类型资源返回true；其他返回false
     */
    @Test
    public void applicable_SapHanaInstConnectionChecker_success() {
        ProtectedResource dbResource = new ProtectedResource();
        dbResource.setSubType(ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        Assert.assertTrue(dbConnChecker.applicable(dbResource));
        ProtectedResource instResource = new ProtectedResource();
        instResource.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        Assert.assertFalse(dbConnChecker.applicable(instResource));
    }

    private ProtectedResource mockClusterDbResource(boolean isModify, boolean isSystemDb) {
        ProtectedResource dbResource = new ProtectedResource();
        dbResource.setUuid("3717e3b69a014535866e91cb6b42949c");
        dbResource.setSubType(ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        dbResource.setParentUuid("589db77f57574583ae311712f5d4c97b");
        if (isSystemDb) {
            dbResource.setExtendInfoByKey(SapHanaConstants.SAP_HANA_DB_TYPE, SapHanaConstants.SYSTEM_DB_TYPE);
        } else {
            dbResource.setExtendInfoByKey(SapHanaConstants.SAP_HANA_DB_TYPE, SapHanaConstants.TENANT_DB_TYPE);
        }
        String nodes = "[{\"uuid\":\"0ed8b119-7d23-475d-9ad3-4fa8e353ed0b\",\"linkStatus\":\"1\"},"
            + "{\"uuid\":\"cfedb495-6574-41e3-843e-e1cb2fc7afd3\",\"linkStatus\":\"1\"}]";
        dbResource.setExtendInfoByKey(SapHanaConstants.NODES, nodes);
        if (isModify) {
            dbResource.setExtendInfoByKey(SapHanaConstants.OPERATION_TYPE, SapHanaConstants.MODIFY_OPERATION_TYPE);
        }
        return dbResource;
    }

    private ProtectedEnvironment mockClusterInstance() {
        ProtectedEnvironment instEnv = new ProtectedEnvironment();
        instEnv.setUuid("589db77f57574583ae311712f5d4c97b");
        instEnv.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        String nodes = "[{\"uuid\":\"0ed8b119-7d23-475d-9ad3-4fa8e353ed0b\",\"linkStatus\":\"1\"},"
            + "{\"uuid\":\"cfedb495-6574-41e3-843e-e1cb2fc7afd3\",\"linkStatus\":\"1\"}]";
        instEnv.setExtendInfoByKey(SapHanaConstants.NODES, nodes);
        return instEnv;
    }

    private List<CheckReport<Object>> mockClusterDbConnCheckReport(boolean isModify, boolean isSystemDb,
        boolean isFirSuccess, boolean isSecSuccess) {
        CheckReport<Object> report = new CheckReport<>();
        ProtectedResource instResource = mockClusterDbResource(isModify, isSystemDb);
        report.setResource(instResource);
        CheckResult<Object> firCheckResult = new CheckResult<>();
        ActionResult firActionResult = new ActionResult();
        if (!isFirSuccess) {
            firActionResult.setCode(IsmNumberConstant.TWO_HUNDRED);
            firActionResult.setBodyErr(String.valueOf(1577213476L));
        }
        firCheckResult.setResults(firActionResult);
        CheckResult<Object> secCheckResult = new CheckResult<>();
        ActionResult secActionResult = new ActionResult();
        if (!isSecSuccess) {
            secActionResult.setCode(IsmNumberConstant.TWO_HUNDRED);
            secActionResult.setBodyErr(String.valueOf(1577213476L));
        }
        secCheckResult.setResults(secActionResult);
        report.setResults(Arrays.asList(firCheckResult, secCheckResult));
        return Collections.singletonList(report);
    }
}
