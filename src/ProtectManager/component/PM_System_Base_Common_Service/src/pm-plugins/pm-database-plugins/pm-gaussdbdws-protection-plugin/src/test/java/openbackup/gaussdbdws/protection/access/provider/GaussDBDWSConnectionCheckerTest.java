package openbackup.gaussdbdws.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.interceptor.backup.MockInterceptorParameter;

import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
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
 * DWS集群环境 联通性测试类
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-17
 */
@RunWith(PowerMockRunner.class)
public class GaussDBDWSConnectionCheckerTest {
    private GaussDBDWSConnectionChecker gaussDBDWSConnectionChecker;

    private AgentUnifiedService agentUnifiedService;

    private final GaussDBBaseService gaussDBBaseService = PowerMockito.mock(GaussDBBaseService.class);

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void init() throws IllegalAccessException {
        ProtectedEnvironmentRetrievalsService environmentRetrievalsService = PowerMockito.mock(
            ProtectedEnvironmentRetrievalsService.class);
        agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);
        this.gaussDBDWSConnectionChecker = new GaussDBDWSConnectionChecker(environmentRetrievalsService,
            agentUnifiedService);
        MemberModifier.field(GaussDBDWSConnectionChecker.class, "gaussDBBaseService")
            .set(gaussDBDWSConnectionChecker, gaussDBBaseService);
    }

    /**
     * 用例场景：GaussDB(DWS)集群环境 联通性provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.GAUSSDB_DWS.getType());
        Assert.assertTrue(gaussDBDWSConnectionChecker.applicable(protectedResource));
    }

    /**
     * 用例场景：GaussDB(DWS)集群环境 联通性校验结果收集器
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void check_generate_check_result_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> stringListMap = Optional.ofNullable(protectedResource.getDependencies())
            .orElse(new HashMap<>());
        List<ProtectedResource> protectedResources = new ArrayList<>();
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("123456");
        protectedResources.add(resource);
        protectedResources.add(resource);
        stringListMap.put(DwsConstant.DWS_CLUSTER_AGENT, protectedResources);
        stringListMap.put(DwsConstant.HOST_AGENT, protectedResources);
        protectedResource.setDependencies(stringListMap);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("123456");
        protectedResource.setEnvironment(protectedEnvironment);
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(agentBaseDto);
        PowerMockito.when(gaussDBBaseService.getEnvironmentById("123456"))
            .thenReturn(MockInterceptorParameter.getProtectedEnvironment());
        gaussDBDWSConnectionChecker.generateCheckResult(protectedResource);
    }

    /**
     * 用例场景：GaussDB(DWS)集群环境 联通性校验结果收集器
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void check_collect_action_results_success() {
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
        Assert.assertEquals(2,gaussDBDWSConnectionChecker.collectActionResults(checkReport,new HashMap<>()).size());
    }
}
