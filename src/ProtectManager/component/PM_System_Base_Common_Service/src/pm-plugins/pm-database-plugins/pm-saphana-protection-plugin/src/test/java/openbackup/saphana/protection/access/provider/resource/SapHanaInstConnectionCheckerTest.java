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
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
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
 * {@link SapHanaInstConnectionChecker Test}
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-05-30
 */
public class SapHanaInstConnectionCheckerTest {
    private final ProtectedEnvironmentRetrievalsService environmentRetrievalsService = PowerMockito.mock(
        ProtectedEnvironmentRetrievalsService.class);

    private final AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final ProtectedEnvironmentService environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);

    private final SapHanaResourceService hanaResourceService = PowerMockito.mock(SapHanaResourceService.class);

    private final SapHanaInstConnectionChecker instConnChecker = new SapHanaInstConnectionChecker(
        environmentRetrievalsService, agentUnifiedService, resourceService, environmentService, hanaResourceService);

    /**
     * 用例场景：收集连通测试需要的资源信息
     * 前置条件：输入连通测试的资源信息
     * 检查点：返回需要的资源信息
     */
    @Test
    public void collectConnectableResources_success() {
        ProtectedResource instResource = mockClusterInstanceResource(false);
        List<ProtectedEnvironment> hostEnvList = mockClusterInstanceHostEnvList();
        PowerMockito.doReturn(hostEnvList)
            .when(hanaResourceService)
            .queryEnvironments(ArgumentMatchers.anyList());
        Map<ProtectedResource, List<ProtectedEnvironment>> resourceListMap
            = instConnChecker.collectConnectableResources(instResource);
        Assert.assertEquals(2, resourceListMap.get(instResource).size());
    }

    /**
     * 用例场景：收集连通测试结果列表
     * 前置条件：输入的检查报告列表为空
     * 检查点：返回空列表
     */
    @Test
    public void test_should_return_empty_list_if_report_list_is_empty_when_collectActionResults() {
        List<CheckReport<Object>> reportList = new ArrayList<>();
        Map<String, Object> context = new HashMap<>();
        List<ActionResult> resultList = instConnChecker.collectActionResults(reportList, context);
        Assert.assertEquals(0, resultList.size());
    }

    /**
     * 用例场景：收集连通测试结果列表
     * 前置条件：注册场景，输入合法的检查报告
     * 检查点：返回测试结果列表
     */
    @Test
    public void test_should_return_non_empty_list_if_register_inst_when_collectActionResults() {
        PowerMockito.when(resourceService.getResourceById("3717e3b69a014535866e91cb6b42949c"))
            .thenReturn(Optional.empty());
        List<CheckReport<Object>> reportList = mockClusterInstConnCheckReport(false, true, false);
        Map<String, Object> context = new HashMap<>();
        List<ActionResult> resultList = instConnChecker.collectActionResults(reportList, context);
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
    public void test_should_return_non_empty_list_if_modify_inst_when_collectActionResults() {
        PowerMockito.when(resourceService.getResourceById("3717e3b69a014535866e91cb6b42949c"))
            .thenReturn(Optional.of(mockClusterInstanceResource(false)));
        List<CheckReport<Object>> reportList = mockClusterInstConnCheckReport(true, false, false);
        ProtectedResource instResourceOfReport = reportList.get(0).getResource();
        PowerMockito.when(hanaResourceService.isModifyResource(instResourceOfReport)).thenReturn(true);
        Map<String, Object> context = new HashMap<>();
        List<ActionResult> resultList = instConnChecker.collectActionResults(reportList, context);
        List<ActionResult> successResultList = resultList.stream()
            .filter(result -> result.getCode() == DatabaseConstants.SUCCESS_CODE)
            .collect(Collectors.toList());
        Assert.assertEquals(2, resultList.size());
        Assert.assertEquals(0, successResultList.size());
    }

    /**
     * 用例场景：收集连通测试结果列表
     * 前置条件：连通测试场景，输入合法的检查报告
     * 检查点：返回测试结果列表
     */
    @Test
    public void test_should_return_non_empty_list_if_conn_inst_when_collectActionResults() {
        PowerMockito.when(resourceService.getResourceById("3717e3b69a014535866e91cb6b42949c"))
            .thenReturn(Optional.of(mockClusterInstanceResource(false)));
        List<CheckReport<Object>> reportList = mockClusterInstConnCheckReport(false, false, true);
        ProtectedResource instResourceOfReport = reportList.get(0).getResource();
        PowerMockito.when(hanaResourceService.isModifyResource(instResourceOfReport)).thenReturn(false);
        ProtectedEnvironment instEnv = mockClusterInstanceEnvironment();
        PowerMockito.doReturn(instEnv).when(environmentService).getEnvironmentById("3717e3b69a014535866e91cb6b42949c");
        PowerMockito.doReturn("1")
            .when(hanaResourceService)
            .getInstStatusByActionResults(ArgumentMatchers.any(ProtectedEnvironment.class), ArgumentMatchers.any());
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .updateInstAndDbLinkStatusByInst(ArgumentMatchers.any(ProtectedEnvironment.class), ArgumentMatchers.eq("1"),
                ArgumentMatchers.eq(true), ArgumentMatchers.eq(false));
        Map<String, Object> context = new HashMap<>();
        List<ActionResult> resultList = instConnChecker.collectActionResults(reportList, context);
        List<ActionResult> successResultList = resultList.stream()
            .filter(result -> result.getCode() == DatabaseConstants.SUCCESS_CODE)
            .collect(Collectors.toList());
        Assert.assertEquals(2, resultList.size());
        Assert.assertEquals(1, successResultList.size());
    }

    /**
     * 用例场景：框架调applicable接口
     * 前置条件：applicable输入资源信息
     * 检查点：SAPHANA-instance类型资源返回true；其他返回false
     */
    @Test
    public void applicable_SapHanaInstConnectionChecker_success() {
        ProtectedResource instResource = new ProtectedResource();
        instResource.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        Assert.assertTrue(instConnChecker.applicable(instResource));
        ProtectedResource dbResource = new ProtectedResource();
        dbResource.setSubType(ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        Assert.assertFalse(instConnChecker.applicable(dbResource));
    }

    private ProtectedResource mockClusterInstanceResource(boolean isModify) {
        ProtectedResource instResource = new ProtectedResource();
        instResource.setUuid("3717e3b69a014535866e91cb6b42949c");
        instResource.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        String nodes = "[{\"uuid\":\"0ed8b119-7d23-475d-9ad3-4fa8e353ed0b\",\"linkStatus\":\"1\"},"
            + "{\"uuid\":\"cfedb495-6574-41e3-843e-e1cb2fc7afd3\",\"linkStatus\":\"1\"}]";
        instResource.setExtendInfoByKey(SapHanaConstants.NODES, nodes);
        if (isModify) {
            instResource.setExtendInfoByKey(SapHanaConstants.OPERATION_TYPE, SapHanaConstants.MODIFY_OPERATION_TYPE);
        }
        return instResource;
    }

    private ProtectedEnvironment mockClusterInstanceEnvironment() {
        ProtectedEnvironment instEnv = new ProtectedEnvironment();
        instEnv.setUuid("3717e3b69a014535866e91cb6b42949c");
        instEnv.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        String nodes = "[{\"uuid\":\"0ed8b119-7d23-475d-9ad3-4fa8e353ed0b\",\"linkStatus\":\"1\"},"
            + "{\"uuid\":\"cfedb495-6574-41e3-843e-e1cb2fc7afd3\",\"linkStatus\":\"1\"}]";
        instEnv.setExtendInfoByKey(SapHanaConstants.NODES, nodes);
        return instEnv;
    }

    private List<ProtectedEnvironment> mockClusterInstanceHostEnvList() {
        ProtectedEnvironment firHostEnv = new ProtectedEnvironment();
        firHostEnv.setUuid("0ed8b119-7d23-475d-9ad3-4fa8e353ed0b");
        firHostEnv.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        ProtectedEnvironment secHostEnv = new ProtectedEnvironment();
        secHostEnv.setUuid("cfedb495-6574-41e3-843e-e1cb2fc7afd3");
        secHostEnv.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        return Arrays.asList(firHostEnv, secHostEnv);
    }

    private List<CheckReport<Object>> mockClusterInstConnCheckReport(boolean isModify, boolean isFirSuccess,
        boolean isSecSuccess) {
        CheckReport<Object> report = new CheckReport<>();
        ProtectedResource instResource = mockClusterInstanceResource(isModify);
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
