package openbackup.oceanbase.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.argThat;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.oceanbase.OceanBaseTest;
import openbackup.oceanbase.common.constants.OBConstants;
import openbackup.oceanbase.common.constants.OBErrorCodeConstants;
import openbackup.oceanbase.common.dto.OBClusterInfo;
import openbackup.oceanbase.common.util.OceanBaseUtils;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatcher;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-24
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {EnvironmentLinkStatusHelper.class})
public class OceanBaseClusterConnectionCheckerTest extends OceanBaseTest {

    private static final String ONLINE = LinkStatusEnum.ONLINE.getStatus().toString();

    private static final String OFFLINE = LinkStatusEnum.OFFLINE.getStatus().toString();

    @MockBean
    private MemberClusterService memberClusterService;

    @Autowired
    private EnvironmentLinkStatusHelper environmentLinkStatusHelper;

    private ProtectedEnvironmentRetrievalsService environmentRetrievalsService;

    private AgentUnifiedService agentUnifiedService;

    private ProviderManager providerManager;

    private OceanBaseClusterConnectionChecker oceanBaseClusterConnectionChecker;

    private UnifiedConnectionCheckProvider unifiedConnectionCheckProvider;

    @Override
    @Before
    public void init() {
        super.init();
        environmentRetrievalsService = Mockito.mock(ProtectedEnvironmentRetrievalsService.class);
        agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
        providerManager = Mockito.mock(ProviderManager.class);
        oceanBaseClusterConnectionChecker = new OceanBaseClusterConnectionChecker(environmentRetrievalsService,
            agentUnifiedService, oceanBaseService);
        unifiedConnectionCheckProvider = new UnifiedConnectionCheckProvider(providerManager,
            oceanBaseClusterConnectionChecker, null);

        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(oceanBaseClusterConnectionChecker);
        PowerMockito.when(providerManager.findProviderOrDefault(any(), any(), any()))
            .thenReturn(oceanBaseClusterConnectionChecker);
        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("esn");

    }

    /**
     * 用例场景：解析出需要检查的agent信息
     * 前置条件：OceanProtect服务正常
     * 检查点：无异常
     */
    @Test
    public void collect_connectable_resources_success() {
        ProtectedResource resource = mockProtectedResource();
        mockGetEvnById();
        Map<ProtectedResource, List<ProtectedEnvironment>> map
            = oceanBaseClusterConnectionChecker.collectConnectableResources(resource);

        Assert.assertEquals(map.size(), 1);
        Assert.assertEquals(map.get(resource).size(), 4);
        List<ProtectedEnvironment> list = map.get(resource);
        Assert.assertEquals(list.get(0).getExtendInfoByKey(OBConstants.KEY_CHECK_TYPE), OBConstants.CHECK_OBSERVER);
        Assert.assertEquals(list.get(1).getExtendInfoByKey(OBConstants.KEY_CHECK_TYPE), OBConstants.CHECK_OBSERVER);
        Assert.assertEquals(list.get(2).getExtendInfoByKey(OBConstants.KEY_CHECK_TYPE), OBConstants.CHECK_OBCLIENT);
        Assert.assertEquals(list.get(3).getExtendInfoByKey(OBConstants.KEY_CHECK_TYPE), OBConstants.CHECK_OBCLIENT);
        Assert.assertEquals(list.get(0).getExtendInfoByKey(OBConstants.KEY_AGENT_IP), "8.40.129.22");
        Assert.assertEquals(list.get(1).getExtendInfoByKey(OBConstants.KEY_AGENT_IP), "8.40.129.23");
        Assert.assertEquals(list.get(2).getExtendInfoByKey(OBConstants.KEY_AGENT_IP), obClient1.getEndpoint());
        Assert.assertEquals(list.get(3).getExtendInfoByKey(OBConstants.KEY_AGENT_IP), obClient2.getEndpoint());
    }

    /**
     * 用例场景：连通性检查成功（注册检查连通性场景）
     * 前置条件：OceanProtect服务正常
     * 检查点：无异常
     */
    @Test
    public void check_register_connect_success() {
        ProtectedResource resource = mockProtectedResource();
        resource.setExtendInfoByKey(OBConstants.KEY_CHECK_SCENE, OBConstants.CLUSTER_REGISTER);
        mockGetEvnById();
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(agentBaseDto);

        ResourceCheckContext context = unifiedConnectionCheckProvider.tryCheckConnection(resource);
        List<ActionResult> check = context.getActionResults();
        Assert.assertEquals(4, check.size());
        Assert.assertEquals(0, check.get(0).getCode());
        Assert.assertEquals(0, check.get(1).getCode());
        Assert.assertEquals(0, check.get(2).getCode());
        Assert.assertEquals(0, check.get(3).getCode());

        verify(agentUnifiedService, times(4)).checkApplication(any(), any());
        verify(oceanBaseService, never()).setTenantSetStatue(any());
        verify(oceanBaseService, never()).updateSourceDirectly(any());
    }

    /**
     * 用例场景：健康检查成功（更新数据库）
     * 前置条件：OceanProtect服务正常
     * 检查点：无异常，所有状态都是ONLINE
     */
    @Test
    public void check_connect_success() {
        ProtectedResource resource = mockProtectedResource();
        mockGetEvnById();
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(agentBaseDto);

        ResourceCheckContext context = unifiedConnectionCheckProvider.tryCheckConnection(resource);
        List<ActionResult> check = context.getActionResults();
        Assert.assertEquals(4, check.size());
        Assert.assertEquals(0, check.get(0).getCode());
        Assert.assertEquals(0, check.get(1).getCode());
        Assert.assertEquals(0, check.get(2).getCode());
        Assert.assertEquals(0, check.get(3).getCode());

        verify(agentUnifiedService, times(4)).checkApplication(any(), any());
        verify(oceanBaseService, never()).setTenantSetStatue(any());
        verify(oceanBaseService).updateSourceDirectly(argThat(allOnline()));
    }

    private ArgumentMatcher<ProtectedEnvironment> allOnline() {
        return new ArgumentMatcher<ProtectedEnvironment>() {
            @Override
            public boolean matches(ProtectedEnvironment argument) {
                // 检查集群的状态应为ONLINE
                if (!Objects.equals(argument.getLinkStatus(), ONLINE)) {
                    return false;
                }
                OBClusterInfo obClusterInfo = OceanBaseUtils.readExtendClusterInfo(argument);
                // 检查所有OBServer的状态应为ONLINE
                if (obClusterInfo.getObServerAgents()
                    .stream()
                    .anyMatch(item -> !Objects.equals(ONLINE, item.getLinkStatus()))) {
                    return false;
                }
                // 检查所有的OBClient的状态应为ONLINE
                if (obClusterInfo.getObClientAgents()
                    .stream()
                    .anyMatch(item -> !Objects.equals(ONLINE, item.getLinkStatus()))) {
                    return false;
                }

                return true;
            }
        };
    }

    /**
     * 用例场景：健康检查，client节点返回认证错误
     * 前置条件：OceanProtect服务正常
     * 检查点：集群离线，client在线， 对应OBServer离线
     */
    @Test
    public void check_health_auth_error() {
        ProtectedResource resource = mockProtectedResource();
        mockGetEvnById();
        AgentBaseDto agentBaseSuccess = new AgentBaseDto();
        agentBaseSuccess.setErrorCode("0");

        // 设置OBClient1检查是发现OBServer1认证失败
        String actionResultStr
            = "{\"code\":1577209942,\"bodyErr\":\"1577209942\",\"message\":\"{\\\"errorCode\\\": 1577209942, \\\"parameters\\\": [\\\"8.40.129.22\\\"], \\\"errorMessage\\\": \\\"Observer 8.40.129.22 auth failed!\\\"}\",\"detailParams\":[\"8.40.129.22\"]}";
        ActionResult authErrorResult = JsonUtil.read(actionResultStr, ActionResult.class);

        AgentBaseDto authError = new AgentBaseDto();
        authError.setErrorCode(String.valueOf(OBErrorCodeConstants.AUTH_ERROR));
        authError.setErrorMessage(JsonUtil.json(authErrorResult));

        PowerMockito.when(agentUnifiedService.checkApplication(any(), any()))
            .thenReturn(agentBaseSuccess)
            .thenReturn(agentBaseSuccess)
            .thenReturn(authError)
            .thenReturn(agentBaseSuccess);

        ResourceCheckContext context = unifiedConnectionCheckProvider.tryCheckConnection(resource);
        List<ActionResult> check = context.getActionResults();
        Assert.assertEquals(4, check.size());
        Assert.assertEquals(0, check.get(0).getCode());
        Assert.assertEquals(0, check.get(1).getCode());
        Assert.assertEquals(OBErrorCodeConstants.AUTH_ERROR, check.get(2).getCode());
        Assert.assertEquals(0, check.get(3).getCode());

        verify(agentUnifiedService, times(4)).checkApplication(any(), any());
        verify(oceanBaseService).setTenantSetStatue(any());
        verify(oceanBaseService).updateSourceDirectly(argThat(matchAuthError()));
    }

    private ArgumentMatcher<ProtectedEnvironment> matchAuthError() {
        return new ArgumentMatcher<ProtectedEnvironment>() {
            @Override
            public boolean matches(ProtectedEnvironment argument) {
                // 检查集群的状态应为OFFLINE
                if (!Objects.equals(argument.getLinkStatus(), OFFLINE)) {
                    return false;
                }
                OBClusterInfo obClusterInfo = OceanBaseUtils.readExtendClusterInfo(argument);
                // 检查接口返回的OBServer1的状态应为OFFLINE
                if (obClusterInfo.getObServerAgents()
                    .stream()
                    .filter(item -> Objects.equals("8.40.129.22", item.getIp()))
                    .noneMatch(item -> Objects.equals(OFFLINE, item.getLinkStatus()))) {
                    return false;
                }
                // 检查其他OBServer的状态应为ONLINE
                if (obClusterInfo.getObServerAgents()
                    .stream()
                    .filter(item -> !Objects.equals("8.40.129.22", item.getIp()))
                    .anyMatch(item -> !Objects.equals(ONLINE, item.getLinkStatus()))) {
                    return false;
                }

                // 检查所有OBClient为ONLINE
                return obClusterInfo.getObClientAgents()
                    .stream()
                    .allMatch(item -> Objects.equals(ONLINE, item.getLinkStatus()));
            }
        };
    }

    /**
     * 用例场景：健康检查，client节点返回集群节点数不一致错误
     * 前置条件：OceanProtect服务正常
     * 检查点：集群离线，client在线， 所有OBServer离线
     */
    @Test
    public void check_health_cluster_node_count_not_same_error() {
        ProtectedResource resource = mockProtectedResource();
        mockGetEvnById();
        AgentBaseDto agentBaseSuccess = new AgentBaseDto();
        agentBaseSuccess.setErrorCode("0");

        // 设置OBClient1检查是发现集群注册的节点数和实际集群不一致
        ActionResult authErrorResult = new ActionResult(OBErrorCodeConstants.CLUSTER_NODE_COUNT_NOT_SAME_ERROR, "");
        authErrorResult.setBodyErr(String.valueOf(OBErrorCodeConstants.CLUSTER_NODE_COUNT_NOT_SAME_ERROR));
        AgentBaseDto authError = new AgentBaseDto();
        authError.setErrorCode(String.valueOf(OBErrorCodeConstants.CLUSTER_NODE_COUNT_NOT_SAME_ERROR));
        authError.setErrorMessage(JsonUtil.json(authErrorResult));

        PowerMockito.when(agentUnifiedService.checkApplication(any(), any()))
            .thenReturn(agentBaseSuccess)
            .thenReturn(agentBaseSuccess)
            .thenReturn(authError)
            .thenReturn(agentBaseSuccess);

        ResourceCheckContext context = unifiedConnectionCheckProvider.tryCheckConnection(resource);
        List<ActionResult> check = context.getActionResults();
        Assert.assertEquals(4, check.size());
        Assert.assertEquals(0, check.get(0).getCode());
        Assert.assertEquals(0, check.get(1).getCode());
        Assert.assertEquals(OBErrorCodeConstants.CLUSTER_NODE_COUNT_NOT_SAME_ERROR, check.get(2).getCode());
        Assert.assertEquals(0, check.get(3).getCode());

        verify(agentUnifiedService, times(4)).checkApplication(any(), any());
        verify(oceanBaseService).setTenantSetStatue(any());
        verify(oceanBaseService).updateSourceDirectly(argThat(matchClusterCountError()));
    }

    private ArgumentMatcher<ProtectedEnvironment> matchClusterCountError() {
        return new ArgumentMatcher<ProtectedEnvironment>() {
            @Override
            public boolean matches(ProtectedEnvironment argument) {
                // 检查集群的状态应为OFFLINE
                if (!Objects.equals(argument.getLinkStatus(), OFFLINE)) {
                    return false;
                }
                OBClusterInfo obClusterInfo = OceanBaseUtils.readExtendClusterInfo(argument);
                // 所有的OBServer的状态应为OFFLINE
                if (obClusterInfo.getObServerAgents()
                    .stream()
                    .noneMatch(item -> Objects.equals(OFFLINE, item.getLinkStatus()))) {
                    return false;
                }
                // 检查所有OBClient为ONLINE
                return obClusterInfo.getObClientAgents()
                    .stream()
                    .allMatch(item -> Objects.equals(ONLINE, item.getLinkStatus()));
            }
        };
    }

    /**
     * 用例场景：健康检查，部分client节点返回agent连接失败
     * 前置条件：OceanProtect服务正常
     * 检查点：集群在线，对应client离线， 所有OBServer在线
     */
    @Test
    public void check_health_agent_network_error() {
        ProtectedResource resource = mockProtectedResource();
        mockGetEvnById();
        AgentBaseDto agentBaseSuccess = new AgentBaseDto();
        agentBaseSuccess.setErrorCode("0");

        // 设置OBClient1检查是agent连接失败
        ActionResult authErrorResult = new ActionResult(CommonErrorCode.AGENT_NETWORK_ERROR, "");
        authErrorResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
        AgentBaseDto authError = new AgentBaseDto();
        authError.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
        authError.setErrorMessage(JsonUtil.json(authErrorResult));

        // 只有1个OBClient离线
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any()))
            .thenReturn(agentBaseSuccess)
            .thenReturn(agentBaseSuccess)
            .thenReturn(authError) // OBClient1检查是agent连接失败
            .thenReturn(agentBaseSuccess);

        ResourceCheckContext context = unifiedConnectionCheckProvider.tryCheckConnection(resource);
        List<ActionResult> check = context.getActionResults();
        Assert.assertEquals(4, check.size());
        Assert.assertEquals(0, check.get(0).getCode());
        Assert.assertEquals(0, check.get(1).getCode());
        // 部分obclient在线，连通性检查要返回成功，所以这里code是0
        Assert.assertEquals(0, check.get(2).getCode());
        Assert.assertEquals(0, check.get(3).getCode());

        verify(agentUnifiedService, times(4)).checkApplication(any(), any());
        verify(oceanBaseService, never()).setTenantSetStatue(any());
        verify(oceanBaseService).updateSourceDirectly(argThat(matchAgentNetworkError(false)));
    }

    /**
     * 用例场景：健康检查，所有client节点返回agent连接失败
     * 前置条件：OceanProtect服务正常
     * 检查点：集群离线，对应client离线， 所有OBServer离线
     */
    @Test
    public void check_health_agent_network_error2() {
        ProtectedResource resource = mockProtectedResource();
        mockGetEvnById();
        AgentBaseDto agentBaseSuccess = new AgentBaseDto();
        agentBaseSuccess.setErrorCode("0");

        // 设置OBClient检查是agent连接失败
        ActionResult authErrorResult = new ActionResult(CommonErrorCode.AGENT_NETWORK_ERROR, "");
        authErrorResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
        AgentBaseDto authError = new AgentBaseDto();
        authError.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
        authError.setErrorMessage(JsonUtil.json(authErrorResult));

        // 所有OBClient离线
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any()))
            .thenReturn(agentBaseSuccess)
            .thenReturn(agentBaseSuccess)
            .thenReturn(authError) // OBClient1返回连接失败
            .thenReturn(authError); // OBClient2返回连接失败

        ResourceCheckContext context2 = unifiedConnectionCheckProvider.tryCheckConnection(resource);
        List<ActionResult> check2 = context2.getActionResults();
        Assert.assertEquals(4, check2.size());
        Assert.assertEquals(0, check2.get(0).getCode());
        Assert.assertEquals(0, check2.get(1).getCode());
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, check2.get(2).getCode());
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, check2.get(3).getCode());

        verify(agentUnifiedService, times(4)).checkApplication(any(), any());
        verify(oceanBaseService).setTenantSetStatue(any());
        verify(oceanBaseService).updateSourceDirectly(argThat(matchAgentNetworkError(true)));
    }

    private ArgumentMatcher<ProtectedEnvironment> matchAgentNetworkError(boolean isAllOBClientOffline) {
        return new ArgumentMatcher<ProtectedEnvironment>() {
            @Override
            public boolean matches(ProtectedEnvironment argument) {
                OBClusterInfo obClusterInfo = OceanBaseUtils.readExtendClusterInfo(argument);
                // 所有的OBServer的状态应为ONLINE
                if (obClusterInfo.getObServerAgents()
                    .stream()
                    .anyMatch(item -> Objects.equals(OFFLINE, item.getLinkStatus()))) {
                    return false;
                }

                if (isAllOBClientOffline) {
                    // 全部OBClient离线，检查集群的状态应为OFFLINE
                    if (!Objects.equals(argument.getLinkStatus(), OFFLINE)) {
                        return false;
                    }
                    // 检查所有OBClient的状态应为OFFINE
                    return obClusterInfo.getObClientAgents()
                        .stream()
                        .allMatch(item -> Objects.equals(OFFLINE, item.getLinkStatus()));

                } else {
                    // 部分OBClient离线，检查集群的状态应为ONLINE
                    if (Objects.equals(argument.getLinkStatus(), OFFLINE)) {
                        return false;
                    }
                    // 检查对应的OBClient1的状态应为OFFLINE
                    if (obClusterInfo.getObClientAgents()
                        .stream()
                        .filter(item -> Objects.equals(obClient1.getUuid(), item.getParentUuid()))
                        .noneMatch(item -> Objects.equals(OFFLINE, item.getLinkStatus()))) {
                        return false;
                    }
                    // 检查其他OBClient的状态应为ONLINE
                    return obClusterInfo.getObClientAgents()
                        .stream()
                        .filter(item -> !Objects.equals(obClient1.getUuid(), item.getParentUuid()))
                        .allMatch(item -> Objects.equals(ONLINE, item.getLinkStatus()));
                }
            }
        };
    }

    /**
     * 用例场景：健康检查，server节点返回agent连接失败
     * 前置条件：OceanProtect服务正常
     * 检查点：集群离线，所有client在线， 对应OBServer离线
     */
    @Test
    public void check_health_observer_network_error() {
        ProtectedResource resource = mockProtectedResource();
        mockGetEvnById();
        AgentBaseDto agentBaseSuccess = new AgentBaseDto();
        agentBaseSuccess.setErrorCode("0");

        // 设置OBServer连接失败
        ActionResult authErrorResult = new ActionResult(CommonErrorCode.AGENT_NETWORK_ERROR, "");
        authErrorResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
        AgentBaseDto authError = new AgentBaseDto();
        authError.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
        authError.setErrorMessage(JsonUtil.json(authErrorResult));

        // 第一个OBServer离线
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any()))
            .thenReturn(authError)
            .thenReturn(agentBaseSuccess);

        ResourceCheckContext context = unifiedConnectionCheckProvider.tryCheckConnection(resource);
        List<ActionResult> check = context.getActionResults();
        Assert.assertEquals(4, check.size());
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, check.get(0).getCode());
        Assert.assertEquals(0, check.get(1).getCode());
        Assert.assertEquals(0, check.get(2).getCode());
        Assert.assertEquals(0, check.get(3).getCode());

        verify(agentUnifiedService, times(4)).checkApplication(any(), any());
        verify(oceanBaseService).setTenantSetStatue(any());
        verify(oceanBaseService).updateSourceDirectly(argThat(matchObserverAgentNetworkError()));
    }

    private ArgumentMatcher<ProtectedEnvironment> matchObserverAgentNetworkError() {
        return new ArgumentMatcher<ProtectedEnvironment>() {
            @Override
            public boolean matches(ProtectedEnvironment argument) {
                // 检查集群的状态应为OFFLINE
                if (!Objects.equals(argument.getLinkStatus(), OFFLINE)) {
                    return false;
                }
                OBClusterInfo obClusterInfo = OceanBaseUtils.readExtendClusterInfo(argument);
                // 对应的OBServer1的状态应为OFFLINE
                if (obClusterInfo.getObServerAgents()
                    .stream()
                    .filter(item -> Objects.equals(obServer1.getUuid(), item.getParentUuid()))
                    .noneMatch(item -> Objects.equals(OFFLINE, item.getLinkStatus()))) {
                    return false;
                }

                // 其他OBServer的状态应为ONLINE
                if (obClusterInfo.getObServerAgents()
                    .stream()
                    .filter(item -> !Objects.equals(obServer1.getUuid(), item.getParentUuid()))
                    .anyMatch(item -> !Objects.equals(ONLINE, item.getLinkStatus()))) {
                    return false;
                }

                // 所有client状态为ONLINE
                return obClusterInfo.getObClientAgents()
                    .stream()
                    .allMatch(item -> Objects.equals(ONLINE, item.getLinkStatus()));
            }
        };
    }

    /**
     * 用例场景：健康检查，检查host的状态就已经OFFLINE
     * 前置条件：OceanProtect服务正常
     * 检查点：集群离线，对应agent离线
     */
    @Test
    public void check_health_host_network_error() {
        // 设置第一个OBServer的host是OFFLINE
        obServer1.setLinkStatus(OFFLINE);
        ProtectedResource resource = mockProtectedResource();
        mockGetEvnById();
        AgentBaseDto agentBaseSuccess = new AgentBaseDto();
        agentBaseSuccess.setErrorCode("0");

        // 其他节点正常
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(agentBaseSuccess);

        ResourceCheckContext context = unifiedConnectionCheckProvider.tryCheckConnection(resource);
        List<ActionResult> check = context.getActionResults();
        Assert.assertEquals(4, check.size());
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, check.get(0).getCode());
        Assert.assertEquals(0, check.get(1).getCode());
        Assert.assertEquals(0, check.get(2).getCode());
        Assert.assertEquals(0, check.get(3).getCode());

        verify(agentUnifiedService, times(3)).checkApplication(any(), any());
        verify(oceanBaseService).setTenantSetStatue(any());
        verify(oceanBaseService).updateSourceDirectly(argThat(matchObserverAgentNetworkError()));
    }
}
