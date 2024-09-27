package openbackup.system.base.util;

import openbackup.system.base.common.constants.StatefulsetConstants;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NetPlaneInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;
import openbackup.system.base.util.BusinessNetworkUtil;

import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.List;

/**
 * 测试获取业务ip信息
 *
 * @author swx1010572
 * @since 2022-03-04
 */
public class BusinessNetworkUtilTest {
    private final InfrastructureRestApi infrastructureRestApi = Mockito.mock(InfrastructureRestApi.class);

    /**
     * 用例名称：验证获取protectengine 相关的业务ip。<br/>
     * 前置条件：业务ip存在基础设施返回的接口中。<br/>
     * check点：<br/>
     * 1、正确返回多个protectengine 相关的业务ip；<br/>
     */
    @Test
    @Ignore
    public void get_backup_ip_success() {
        InfraResponseWithError<List<NodePodInfo>> infra = new InfraResponseWithError<>();
        infra.setData(getNodePodInfo());
        PowerMockito.when(infrastructureRestApi.getCollectNetPlaneInfo(StatefulsetConstants.PROTECTENGINE))
            .thenReturn(infra);
        String backupIps = BusinessNetworkUtil.getBackupIp(infrastructureRestApi).orElse("");
        Assert.assertNotNull(backupIps);
    }

    /**
     * 用例名称：根据NodePodInfo获取node ip信息
     * check点：<br/>
     * 1、正确返回网络平面对应的IP；<br/>
     */
    @Test
    public void test_parse_net_plane() {
        List<NodePodInfo> nodePodInfo = getNodePodInfo();
        String ipList = BusinessNetworkUtil.parseNetPlane(nodePodInfo.get(0), "backupNetPlane");
        Assert.assertEquals("192.168.100.100", ipList);
        String ipList2 = BusinessNetworkUtil.parseNetPlane(nodePodInfo.get(1), "backupNetPlane");
        Assert.assertEquals("192.168.100.101", ipList2);
    }

    /**
     * 用例名称：验证获取protectengine 相关的业务ip。<br/>
     * 前置条件：基础设施返回的接口中不存在任何信息。<br/>
     * check点：<br/>
     * 1、返回值为空；<br/>
     */
    @Test
    public void get_no_backup_ip_success() {
        InfraResponseWithError<List<NodePodInfo>> infra = new InfraResponseWithError<>();
        infra.setData(null);
        PowerMockito.when(infrastructureRestApi.getCollectNetPlaneInfo(StatefulsetConstants.PROTECTENGINE))
            .thenReturn(infra);
        String backupIps = BusinessNetworkUtil.getBackupIp(infrastructureRestApi).orElse("");
        Assert.assertEquals("", backupIps);
    }

    private List<NodePodInfo> getNodePodInfo() {
        List<NodePodInfo> nodePodInfos = new ArrayList<>();
        for (int i = 0; i < 2; i++) {
            NodePodInfo nodePodInfo = new NodePodInfo();
            nodePodInfo.setPodName("protectengine-" + i);
            List<NetPlaneInfo> netPlaneInfos = new ArrayList<>();
            NetPlaneInfo netPlaneInfo1 = new NetPlaneInfo();
            netPlaneInfo1.setNetPlaneName("k8s.v1.cni.cncf.io/networks-status");
            netPlaneInfo1.setIpAddress(
                "[{\"name\": \"default/1\",\n    \"ips\": [\n        \"192.168.100.10" + i + "\"\n    ]}]");
            NetPlaneInfo netPlaneInfo2 = new NetPlaneInfo();
            netPlaneInfo2.setNetPlaneName("backupNetPlane");
            netPlaneInfo2.setIpAddress("1");
            netPlaneInfos.add(netPlaneInfo1);
            netPlaneInfos.add(netPlaneInfo2);
            nodePodInfo.setNetPlaneInfos(netPlaneInfos);
            nodePodInfos.add(nodePodInfo);
        }
        return nodePodInfos;
    }
}
