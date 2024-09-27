package openbackup.exchange.protection.access.provider;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.exchange.protection.access.service.ExchangeService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;

import org.junit.Assert;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

@ExtendWith(MockitoExtension.class)
class ExchangeSingleNodeGroupProviderTest {

    @Mock
    private ProviderManager mockProviderManager;

    @Mock
    private PluginConfigManager mockPluginConfigManager;

    @Mock
    private ExchangeService mockExchangeService;

    @Mock
    private AgentUnifiedService mockAgentUnifiedService;

    private ExchangeSingleNodeProvider exchangeGroupProviderUnderTest;

    @BeforeEach
    void setUp() {
        exchangeGroupProviderUnderTest = new ExchangeSingleNodeProvider(mockProviderManager, mockPluginConfigManager,
            mockAgentUnifiedService, mockExchangeService);
    }

    @Test
    void testApplicable() {
        assertThat(exchangeGroupProviderUnderTest.applicable("resourceSubType")).isFalse();
    }

    @Test
    void testCheck() {
        // Setup
        final ProtectedEnvironment environment = getEnvironment();
        System.out.println(JsonUtil.json(environment));
        ProtectedEnvironment agentEnvironment = getOldEnvironment();
        agentEnvironment.setEndpoint("8.8.8.8");
        agentEnvironment.setPort(666);
        when(mockExchangeService.getEnvironmentById(any())).thenReturn(agentEnvironment);
        when(mockExchangeService.getResourceById(any()))
            .thenThrow(new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
        // Run the test
        exchangeGroupProviderUnderTest.register(environment);

        // Verify the results
    }

    @Test
    void testScan() {
        // Setup
        final ProtectedEnvironment environment = getEnvironment();

        final ProtectedResource resource = new ProtectedResource();
        resource.setName("database");
        resource.setExtendInfoByKey("db_uuid", "666");
        resource.setExtendInfoByKey("db_name", "my_db");
        resource.setExtendInfoByKey("version", "1.5.0");
        final List<ProtectedResource> expectedResult = Collections.singletonList(resource);
        PageListResponse pageListResponse = new PageListResponse();
        pageListResponse.setRecords(expectedResult);
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint("8.8.8.8");
        agentEnvironment.setPort(666);
        when(mockAgentUnifiedService.getDetailPageList(any(), any(), any(), any())).thenReturn(pageListResponse);
        when(mockExchangeService.scanMailboxes(any(), any(), any())).thenReturn(expectedResult);
        // Run the test
        final List<ProtectedResource> result = exchangeGroupProviderUnderTest.scan(environment);
        List<ProtectedResource> expected = new ArrayList<>();
        expected.addAll(expectedResult);
        expected.addAll(expectedResult);
        // Verify the results
        assertThat(result).isEqualTo(expected);
        // 验证环境已删除的场景
        when(mockExchangeService.getEnvironmentById(any()))
            .thenThrow(new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
        List<ProtectedResource>  result2 = exchangeGroupProviderUnderTest.scan(environment);
        assertThat(result2).isEqualTo(new ArrayList<>());
    }

    @Test
    void testHealthCheck() {
        ProtectedEnvironment environment = getEnvironment();
        PowerMockito.doThrow(new LegoCheckedException(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT, "time out"))
            .when(mockExchangeService)
            .checkConnection(any());
        // Run the test
        try {
            exchangeGroupProviderUnderTest.validate(environment);
            Assert.fail();
        } catch (LegoCheckedException exception) {
            Assert.assertEquals(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT, exception.getErrorCode());
        }
    }

    /**
     * 用例场景：校验注册环境名称
     * 前置条件：无
     * 检查点：非法名称是否抛出异常
     */
    @Test
    void testCheckName() {
        List<ProtectedEnvironment> environments = new ArrayList<>();
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("abc$");
        environments.add(environment);
        environment.setName("Abc$");
        environments.add(environment);
        environment.setName("_abc$");
        environments.add(environment);
        environment.setName("我abc$");
        environments.add(environment);
        environment.setName("$abc");
        environments.add(environment);
        environment.setName(null);
        environments.add(environment);
        environment.setName("    ");
        environments.add(environment);
        environment.setName("");
        environments.add(environment);
        for (ProtectedEnvironment protectedEnvironment : environments) {
            try {
                exchangeGroupProviderUnderTest.register(protectedEnvironment);
            } catch (LegoCheckedException e) {
                Assertions.assertEquals("Illegal name.", e.getMessage());
            }
        }
    }

    private ProtectedEnvironment getEnvironment() {
        String json =
            "{\"name\":\"Server01\",\"type\":\"Database\",\"subType\":\"Exchange-group\",\"extendInfo\":{\"linkStatus\":\"0\",\"agentUuid\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\"\"isGroup\":0},\"auth\":{\"authType\":2,\"authKey\":\"extest\\\\Administrator\",\"authPwd\":\"xxxxx\"},\"dependencies\":{\"agents\":[{\"uuid\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\"}]}}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }

    private ProtectedEnvironment getOldEnvironment() {
        String json =
            "{\"uuid\":\"045c5f35-8007-3aa6-a0c5-35e7c759d4e5\",\"name\":\"见证231_3\",\"type\":\"Database\",\"subType\":\"Exchange-group\",\"createdTime\":\"2023-06-01 17:48:42.351\",\"rootUuid\":\"045c5f35-8007-3aa6-a0c5-35e7c759d4e5\",\"sourceType\":\"register\",\"protectionStatus\":0,\"extendInfo\":{\"isGroup\":\"0\",\"agentUuid\":\"0916a7fc-1300-4a37-9d9d-ea647f443f57\"},\"endpoint\":\"xxxx\",\"port\":0,\"auth\":{\"authType\":2,\"authKey\":\"test\\\\Administrator\",\"authPwd\":\"\",\"extendInfo\":{}},\"dependencies\":{\"agents\":[{\"uuid\":\"2615222a-1993-4f84-8be5-7c74dc335a6d\",\"name\":\"2016MB01\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-29 12:53:53.0\",\"rootUuid\":\"0916a7fc-1300-4a37-9d9d-ea647f443f57\",\"version\":\"1.5.RC1.017\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.99.231,8.40.99.231,fe80:0000:0000:0000:bc77:339e:40e6:0951,fe80:0000:0000:0000:34b1:8ca0:1ef8:1eb2\",\"agentId\":\"1663055121371631618\",\"scenario\":\"0\",\"src_deduption\":\"false\",\"agentUpgradeable\":\"1\",\"agentUpgradeableVersion\":\"1.5.RC1.020\",\"$citations_agents_d63c1b7e67e341829bd3df55c6829395\":\"045c5f35-8007-3aa6-a0c5-35e7c759d4e5\"},\"endpoint\":\"xxxx\",\"port\":59521,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"windows\",\"scanInterval\":3600,\"cluster\":false}]},\"linkStatus\":\"1\",\"scanInterval\":3600,\"cluster\":false}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }
}
