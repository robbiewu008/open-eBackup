package openbackup.clickhouse.plugin.provider;

import openbackup.clickhouse.plugin.constant.ClickHouseConstant;
import openbackup.clickhouse.plugin.service.ClickHouseService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;

/**
 * ClickHouseConnectionChecker Test
 *
 * @author q00464130
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-13
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {ClickHouseConnectionChecker.class, ClickHouseService.class})
public class ClickHouseConnectionCheckerTest {
    @InjectMocks
    private ClickHouseConnectionChecker clickHouseConnectionChecker;

    @Mock
    private AgentUnifiedService agentUnifiedService;

    @Mock
    private ClickHouseService clickHouseService;

    @Before
    public void init() {
        // @Mock注不进去clickHouseService，手动设置下
        ReflectionTestUtils.setField(clickHouseConnectionChecker, "clickHouseService", clickHouseService);
    }

    /**
     * 用例场景：clickHouse类型识别
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Assert.assertTrue(clickHouseConnectionChecker.applicable(protectedResource));
    }

    /**
     * 用例场景：获取检查结果
     * 前置条件：连通性检查成功
     * 检查点： 返回检查结果成功
     */
    @Test
    public void collect_action_results_success() {
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(0);
        actionResult.setMessage("success");
        List<CheckReport<Object>> checkReports = new ArrayList<>();
        CheckReport<Object> checkReport = new CheckReport<>();
        List<CheckResult<Object>> results = new ArrayList<>();
        CheckResult<Object> result = new CheckResult<>();
        results.add(result);
        result.setResults(actionResult);
        checkReport.setResults(results);
        checkReports.add(checkReport);
        List<ActionResult> resultList = clickHouseConnectionChecker.collectActionResults(checkReports, new HashMap<>());
        Assert.assertEquals(resultList.size(), 1);
    }

    /**
     * 用例场景：获取检查结果
     * 前置条件：连通性检查失败
     * 检查点： 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_generate_check_result_failed() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setName("192_168_143_104_22401");
        protectedResource.setUuid("123");
        protectedResource.setType(ClickHouseConstant.NODE_TYPE);
        protectedResource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplicationNoRetry(ArgumentMatchers.any(), ArgumentMatchers.any()))
            .thenReturn(agentBaseDto);
        protectedResource.setExtendInfo(new HashMap<String, String>() {
            {
                put("kerberosId", "15cf44359b63400cbbca9dfe167ae01b");
            }
        });
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        protectedResource.setEnvironment(protectedEnvironment);
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.NO_AUTH);
        auth.setExtendInfo(new HashMap<String, String>() {
            {
                put("kerberosId", "15cf44359b63400cbbca9dfe167ae01b");
            }
        });
        protectedResource.setAuth(auth);
        protectedResource.setExtendInfo(new HashMap<String, String>() {
            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ProtectedResource host1 = new ProtectedResource();
        host1.setUuid(UUID.randomUUID().toString());
        protectedResource.setDependencies(Collections.singletonMap("agents", Arrays.asList(host1)));
        CheckResult<Object> result = clickHouseConnectionChecker.generateCheckResult(protectedResource);
        Assert.assertEquals(0L, result.getResults().getCode());
    }

    /**
     * 用例场景：获取检查结果
     * 前置条件：连通性检查失败
     * 检查点： 抛出异常
     */
    @Test
    public void collectConnectableResources_success() throws Exception {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("123");
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplication(ArgumentMatchers.any(), ArgumentMatchers.any()))
            .thenReturn(agentBaseDto);
        protectedResource.setExtendInfo(new HashMap<String, String>() {
            {
                put("kerberosId", "15cf44359b63400cbbca9dfe167ae01b");
            }
        });
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.NO_AUTH);
        auth.setExtendInfo(new HashMap<String, String>() {
            {
                put("kerberosId", "15cf44359b63400cbbca9dfe167ae01b");
            }
        });
        protectedResource.setAuth(auth);
        ProtectedResource son1 = new ProtectedResource();
        son1.setName("xxx1");
        son1.setUuid("123");
        ProtectedResource son2 = new ProtectedResource();
        son2.setName("xxx1");
        son2.setUuid("456");
        auth.setAuthType(Authentication.NO_AUTH);
        son1.setAuth(auth);
        son2.setAuth(auth);
        son1.setDependencies(Collections.singletonMap(DatabaseConstants.AGENTS, Lists.newArrayList(son2)));
        protectedResource.setDependencies(ImmutableMap.of(ResourceConstants.CHILDREN, Arrays.asList(son1)));
        ProtectedEnvironmentService environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);
        PowerMockito.when(environmentService.getEnvironmentById(ArgumentMatchers.anyString()))
            .thenReturn(new ProtectedEnvironment());
        Class<?> classType = clickHouseConnectionChecker.getClass();
        Field a = classType.getDeclaredField("environmentService");
        a.setAccessible(true);
        a.set(clickHouseConnectionChecker, environmentService);
        clickHouseConnectionChecker.collectConnectableResources(protectedResource);
    }
}