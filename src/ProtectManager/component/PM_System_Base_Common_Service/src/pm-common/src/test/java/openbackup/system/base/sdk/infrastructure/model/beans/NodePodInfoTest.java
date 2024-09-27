package openbackup.system.base.sdk.infrastructure.model.beans;

import openbackup.system.base.sdk.infrastructure.model.beans.NetPlaneInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;

import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;

/**
 * NodePodInfo test
 *
 * @author jwx701567
 * @since 2021-03-12
 */
public class NodePodInfoTest {


    @Test
    public void get_all_ips_success() {
        NodePodInfo nodePodInfo = new NodePodInfo();
        List<NetPlaneInfo> netPlaneInfos = Arrays.asList(
                new NetPlaneInfo("197.0.0.1", "testName"),
                new NetPlaneInfo("199.0.0.1", ""));
        nodePodInfo.setNetPlaneInfos(netPlaneInfos);
        List<String> allIps = nodePodInfo.getAllIps();
        Assert.assertEquals(2, allIps.size());
    }
}
