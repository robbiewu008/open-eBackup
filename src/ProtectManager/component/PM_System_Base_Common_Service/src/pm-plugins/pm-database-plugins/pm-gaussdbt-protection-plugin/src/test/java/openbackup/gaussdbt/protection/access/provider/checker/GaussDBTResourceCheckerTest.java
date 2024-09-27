package openbackup.gaussdbt.protection.access.provider.checker;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * GaussDBT资源检查checker测试类
 *
 * @author hwx1144169
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-31
 */
public class GaussDBTResourceCheckerTest {
    private final UnifiedClusterResourceIntegrityChecker integrityChecker = PowerMockito.mock(UnifiedClusterResourceIntegrityChecker.class);
    private final UnifiedResourceConnectionChecker connectionChecker = PowerMockito.mock(UnifiedResourceConnectionChecker.class);
    private final ProtectedEnvironmentRetrievalsService environmentRetrievalsService = PowerMockito.mock(ProtectedEnvironmentRetrievalsService.class);
    private final GaussDBTResourceChecker gaussDbtResourceChecker = new GaussDBTResourceChecker(environmentRetrievalsService, connectionChecker, integrityChecker);

    /**
     * 用例场景：gaussDBT类型识别
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.GAUSSDBT.getType());
        Assert.assertTrue(gaussDbtResourceChecker.applicable(protectedResource));
    }

    /**
     * 用例场景：获取检查结果
     * 前置条件：检查成功
     * 检查点：返回结果
     */
    @Test
    public void generate_check_result_success_when_integrity_check_success() {
        CheckResult<Object> checkResult = new CheckResult<>();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(ActionResult.SUCCESS_CODE);
        checkResult.setResults(actionResult);
        PowerMockito.when(connectionChecker.generateCheckResult(any())).thenReturn(checkResult);
        CheckResult<AppEnvResponse> integrityResult = new CheckResult<>();
        integrityResult.setData(new AppEnvResponse());
        integrityResult.setEnvironment((ProtectedEnvironment) getProtectedResource());
        PowerMockito.when(integrityChecker.generateCheckResult(any())).thenReturn(integrityResult);
        CheckResult<AppEnvResponse> result = gaussDbtResourceChecker.generateCheckResult(getProtectedResource());
        Assert.assertEquals(ActionResult.SUCCESS_CODE, result.getResults().getCode());
    }

    /**
     * 用例场景：获取检查结果
     * 前置条件：检查失败
     * 检查点：返回结果
     */
    @Test
    public void generate_check_result_success_when_integrity_check_failed() {
        CheckResult<Object> checkResult = new CheckResult<>();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(ActionResult.SUCCESS_CODE);
        checkResult.setResults(actionResult);
        PowerMockito.when(connectionChecker.generateCheckResult(any())).thenReturn(checkResult);
        PowerMockito.doThrow(new LegoCheckedException("")).when(integrityChecker).generateCheckResult(any());
        CheckResult<AppEnvResponse> result = gaussDbtResourceChecker.generateCheckResult(getProtectedResource());
        Assert.assertNull(result.getData());
        Assert.assertEquals(ActionResult.SUCCESS_CODE, result.getResults().getCode());
    }

    /**
     * 用例场景：收集检查结果
     * 前置条件：没有成功的检查
     * 检查点：返回失败的检查
     */
    @Test
    public void will_return_failed_action_results_collect_action_results_when_results_has_no_success() {
        Map<String, Object> context = new HashMap<>();
        List<ActionResult> results = gaussDbtResourceChecker.collectActionResults(getCheckReportsWithNoSuccess(), context);
        Assert.assertEquals(3, results.size());
        Assert.assertEquals(200, results.get(0).getCode());
        Assert.assertNull(context.get(GaussDBTConstant.CLUSTER_INFO_KEY));
    }

    /**
     * 用例场景：收集检查结果
     * 前置条件：有部分成功的检查
     * 检查点：返回成功的检查，并拼装上下文
     */
    @Test
    public void should__throw_LegoCheckedException_collect_action_results_when_results_has_success_but_data_is_null() {
        List<CheckReport<AppEnvResponse>> checkReports = getCheckReportsWithPartialsSuccess();
        checkReports.get(0).getResults().get(1).setData(null);
        Assert.assertThrows(LegoCheckedException.class, () -> gaussDbtResourceChecker.collectActionResults(checkReports, new HashMap<>()));
    }

    /**
     * 用例场景：收集检查结果
     * 前置条件：有部分成功的检查，但是没有数据
     * 检查点：抛出异常
     */
    @Test
    public void will_return_failed_action_results_collect_action_results_when_results_has_partials_success() {
        Map<String, Object> context = new HashMap<>();
        List<ActionResult> results = gaussDbtResourceChecker.collectActionResults(getCheckReportsWithPartialsSuccess(), context);
        Assert.assertEquals(1, results.size());
        Assert.assertEquals(ActionResult.SUCCESS_CODE, results.get(0).getCode());
        Assert.assertNotNull(context.get(GaussDBTConstant.CLUSTER_INFO_KEY));
    }

    private ProtectedResource getProtectedResource() {
        ProtectedResource protectedResource = new ProtectedEnvironment();
        protectedResource.setUuid(UUID.randomUUID().toString());
        protectedResource.setName("test_name");
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource.setSubType(ResourceSubTypeEnum.GAUSSDBT.getType());
        protectedResource.setParentUuid(UUID.randomUUID().toString());
        protectedResource.setParentName("test_parent_name");
        Authentication authentication = new Authentication();
        authentication.setAuthType(Authentication.NO_AUTH);
        protectedResource.setAuth(authentication);
        return protectedResource;
    }

    private List<CheckReport<AppEnvResponse>> getCheckReportsWithNoSuccess() {
        CheckReport<AppEnvResponse> checkReport = new CheckReport<>();
        checkReport.setResource(getProtectedResource());
        CheckResult<AppEnvResponse> checkResult = new CheckResult<>();
        checkResult.setEnvironment((ProtectedEnvironment) getProtectedResource());
        checkResult.setData(null);
        ActionResult actionResult = getActionResult();
        actionResult.setCode(200);
        checkResult.setResults(actionResult);
        CheckResult<AppEnvResponse> checkResult1 = BeanTools.copy(checkResult, CheckResult::new);
        CheckResult<AppEnvResponse> checkResult2 = BeanTools.copy(checkResult, CheckResult::new);
        checkReport.setResults(Arrays.asList(checkResult, checkResult1, checkResult2));
        return Collections.singletonList(checkReport);
    }

    private List<CheckReport<AppEnvResponse>> getCheckReportsWithPartialsSuccess() {
        CheckReport<AppEnvResponse> checkReport = new CheckReport<>();
        checkReport.setResource(getProtectedResource());
        CheckResult<AppEnvResponse> checkResult = new CheckResult<>();
        checkResult.setEnvironment((ProtectedEnvironment) getProtectedResource());
        checkResult.setData(null);
        ActionResult actionResult = getActionResult();
        actionResult.setCode(200);
        checkResult.setResults(actionResult);
        CheckResult<AppEnvResponse> checkResult1 = BeanTools.copy(checkResult, CheckResult::new);
        checkResult1.setResults(getActionResult());
        checkResult1.setData(getAppEnvResponse());
        checkResult1.setEnvironment((ProtectedEnvironment) getProtectedResource());
        CheckResult<AppEnvResponse> checkResult2 = BeanTools.copy(checkResult, CheckResult::new);
        checkReport.setResults(Arrays.asList(checkResult, checkResult1, checkResult2));
        return Collections.singletonList(checkReport);
    }

    private ActionResult getActionResult() {
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(ActionResult.SUCCESS_CODE);
        return actionResult;
    }

    private AppEnvResponse getAppEnvResponse() {
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        NodeInfo nodeInfo1 = new NodeInfo();
        nodeInfo1.setUuid("node01");
        NodeInfo nodeInfo2 = BeanTools.copy(nodeInfo1, NodeInfo::new);
        nodeInfo2.setUuid("node02");
        NodeInfo nodeInfo3 = BeanTools.copy(nodeInfo1, NodeInfo::new);
        nodeInfo3.setUuid("node03");
        appEnvResponse.setNodes(Arrays.asList(nodeInfo1, nodeInfo2, nodeInfo3));
        return appEnvResponse;
    }
}
