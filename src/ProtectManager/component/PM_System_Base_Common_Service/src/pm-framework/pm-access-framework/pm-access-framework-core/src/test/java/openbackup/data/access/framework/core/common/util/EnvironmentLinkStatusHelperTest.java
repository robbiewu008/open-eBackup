package openbackup.data.access.framework.core.common.util;

import static openbackup.data.protection.access.provider.sdk.backup.ResourceExtendInfoConstants.CONNECTION_RESULT_KEY;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * AgentLinkStatusHelper 测试类
 *
 * @author y30044273
 * @since 2023-07-27
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {EnvironmentLinkStatusHelper.class})
public class EnvironmentLinkStatusHelperTest {
    @MockBean
    private MemberClusterService memberClusterService;

    @Autowired
    private EnvironmentLinkStatusHelper environmentLinkStatusHelper;

    @Before
    public void before() {
        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("esn");
    }

    /**
     * 用例场景：正常获取环境连通性
     * 前置条件：非集群场景
     * 检查点：获取连通性正常
     */
    @Test
    public void get_link_status_success_in_common_case() {
        Assert.assertEquals(prepareProtectedEnvironment().getLinkStatus(),
            EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(prepareProtectedEnvironment()));
    }

    /**
     * 用例场景：正常获取环境连通性
     * 前置条件：集群场景
     * 检查点：获取连通性正常
     */
    @Test
    public void get_link_status_success_in_multi_cluster_case() {
        PowerMockito.when(memberClusterService.clusterEstablished()).thenReturn(true);
        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("esn");
        Assert.assertEquals("0",
            EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(prepareProtectedEnvironment()));
    }

    /**
     * 用例场景：获取环境连通性失败
     * 前置条件：集群场景 json格式错误
     * 检查点：返回原有link status字段
     */
    @Test
    public void get_link_status_failed_while_param_incorrect() {
        PowerMockito.when(memberClusterService.clusterEstablished()).thenReturn(true);
        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("111");
        ProtectedEnvironment protectedEnvironment = prepareProtectedEnvironment();
        protectedEnvironment.setExtendInfoByKey(CONNECTION_RESULT_KEY, "{\"esn\":{\"link_status\":\"ok\"}}");
        Assert.assertEquals("123", EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(protectedEnvironment));
    }

    private ProtectedEnvironment prepareProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setLinkStatus("123");
        String result = "{\"esn\":{\"link_status\":\"0\"}}";
        protectedEnvironment.setExtendInfoByKey(CONNECTION_RESULT_KEY, result);
        return protectedEnvironment;
    }

}