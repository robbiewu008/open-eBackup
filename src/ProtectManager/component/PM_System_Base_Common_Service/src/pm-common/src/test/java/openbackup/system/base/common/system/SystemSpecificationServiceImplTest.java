package openbackup.system.base.common.system;

import openbackup.system.base.common.system.SystemSpecificationServiceImpl;
import openbackup.system.base.sdk.SystemSpecificationService;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.service.DeployTypeService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Arrays;

/**
 * System Specification Service Impl Test
 *
 * @author l00272247
 * @since 2021-03-17
 */
@RunWith(SpringRunner.class)
@SpringBootTest
@ContextConfiguration(classes = SystemSpecificationServiceImpl.class)
@MockBean(ClusterNativeApi.class)
public class SystemSpecificationServiceImplTest {
    @MockBean
    private ClusterInternalApi clusterInternalApi;
    @Autowired
    private SystemSpecificationService systemSpecificationService;

    @MockBean
    private InfrastructureRestApi infrastructureRestApi;

    @MockBean
    private DeployTypeService deployTypeService;

    @Test
    public void test() {
        mockClusterInternalApi();
        Assert.assertEquals(1, systemSpecificationService.getClusterNodeCount());
        Assert.assertEquals(20, systemSpecificationService.getSingleNodeJobMaximumConcurrency());
    }

    private void mockClusterInternalApi() {
        SourceClustersParams sourceClustersParams = new SourceClustersParams();
        sourceClustersParams.setMgrIpList(Arrays.asList("1", "2"));
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();
        clusterDetailInfo.setSourceClusters(sourceClustersParams);
        PowerMockito.when(clusterInternalApi.queryClusterDetails()).thenReturn(clusterDetailInfo);
    }
}
