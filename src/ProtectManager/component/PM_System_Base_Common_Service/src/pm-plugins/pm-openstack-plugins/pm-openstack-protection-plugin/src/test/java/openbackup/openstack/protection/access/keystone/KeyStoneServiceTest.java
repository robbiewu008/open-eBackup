/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.openstack.protection.access.keystone;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.constant.KeyStoneConstant;
import openbackup.openstack.protection.access.keystone.util.KeyStoneHttpUtil;
import openbackup.openstack.protection.access.provider.MockFactory;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.util.SpringBeanUtils;

import okhttp3.MediaType;
import okhttp3.Protocol;
import okhttp3.Request;
import okhttp3.Response;
import okhttp3.ResponseBody;

import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.mockito.Spy;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * 功能描述: test KeyStoneService
 *
 * @author x30038064
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-27
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({KeyStoneHttpUtil.class, KeyStoneService.class, SpringBeanUtils.class})
@PowerMockIgnore("javax.net.ssl.*")
@Ignore
public class KeyStoneServiceTest {
    @Spy
    private KeyStoneService keyStoneService = new KeyStoneService(resourceService);

    private static final ResourceService resourceService = Mockito.mock(ResourceService.class);

    /**
     * 测试场景：注册到OpenStack成功 <br/>
     * 前置条件：输入信息正确 <br/>
     * 检查点：是否调用创建endpoint方法
     */
    @Test
    public void test_register_openstack_success() throws Exception {
        Response tokenResponse = mockResponseBuilder().build();
        PowerMockito.mockStatic(KeyStoneHttpUtil.class);
        Mockito.when(KeyStoneHttpUtil.syncPostRequest(Mockito.anyString(), Mockito.anyString(), Mockito.anyString()))
            .thenReturn(tokenResponse);
        Mockito.when(KeyStoneHttpUtil.syncGetRequest(Mockito.anyString(), Mockito.anyMap(), Mockito.anyString(),
                Mockito.anyString()))
            .thenReturn(mockRegionCheckResponse())
            .thenReturn(mockServiceResponse(true))
            .thenReturn(mockEndpointCheckResponse());
        Mockito.when(KeyStoneHttpUtil.syncPostRequest(Mockito.anyString(), Mockito.anyString(), Mockito.anyString(),
                Mockito.anyString()))
            .thenReturn(mockServiceResponse(false))
            .thenReturn(tokenResponse);
        PowerMockito.mockStatic(SpringBeanUtils.class);
        ClusterInternalApi clusterInternalApi = Mockito.mock(ClusterInternalApi.class);
        Mockito.when(clusterInternalApi.queryClusterDetails()).thenReturn(mockClusterDetailInfo());
        Mockito.when(SpringBeanUtils.getBean(Mockito.eq(ClusterInternalApi.class)))
            .thenReturn(clusterInternalApi);

        keyStoneService.registerOpenstack(MockFactory.mockEnvironment());
        // 检查是否调用创建endpoint方法
        PowerMockito.verifyPrivate(keyStoneService, Mockito.times(1))
            .invoke("createEndpoint", Mockito.any(), Mockito.any(), Mockito.any(), Mockito.any());
    }

    /**
     * 测试场景：注册校验项目token成功 <br/>
     * 前置条件：输入信息正确 <br/>
     * 检查点：返回项目id
     */
    @Test
    public void test_verify_project_token_success() {
        PowerMockito.mockStatic(KeyStoneHttpUtil.class);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(Collections.singletonList(MockFactory.mockEnvironment()));
        Mockito.when(resourceService.query(Mockito.anyInt(), Mockito.anyInt(), Mockito.any()))
            .thenReturn(response);
        Mockito.when(KeyStoneHttpUtil.syncGetRequestWithHeader(Mockito.anyString(), Mockito.anyMap()))
            .thenReturn(mockTokenCheckResponse());
        String projectId = keyStoneService.verifyProjectToken("project_token_mock");
        Assert.assertEquals("project_id", projectId);
    }

    private ClusterDetailInfo mockClusterDetailInfo() {
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();
        SourceClustersParams sourceClusters = new SourceClustersParams();
        List<String> pmIps = Arrays.asList("1.1.1.1","1.1.1.2");
        sourceClusters.setMgrIpList(pmIps);
        clusterDetailInfo.setSourceClusters(sourceClusters);
        return clusterDetailInfo;
    }

    private Response mockTokenCheckResponse() {
        String regions = "{\"token\":{\"description\":\"\",\"id\":\"\"," +
            "\"project\":{\"id\":\"project_id\"},\"parent_region_id\":null}}";
        return mockResponseBuilder()
            .body(ResponseBody.create(MediaType.get("application/json"), regions))
            .build();
    }

    private Response mockRegionCheckResponse() {
        String regions = "{\"regions\":[{\"description\":\"\",\"id\":\"RegionOne\"," +
            "\"links\":{\"self\":\"https://example.com\"},\"parent_region_id\":null}]}";
        return mockResponseBuilder()
            .body(ResponseBody.create(MediaType.get("application/json"), regions))
            .build();
    }

    private Response mockServiceResponse(boolean isCheckExist) {
        String serviceCheck = "{\"services\":[]}";
        String service = "{\"service\":{\"description\":\"Nova\",\"enabled\":true," +
            "\"id\":\"1999c3a858c7408fb586817620695098\",\"name\":\"nova\",\"type\":\"compute\"}}";
        if (isCheckExist){
            return mockResponseBuilder()
                .body(ResponseBody.create(MediaType.get("application/json"), serviceCheck))
                .build();
        }
        return mockResponseBuilder()
            .body(ResponseBody.create(MediaType.get("application/json"), service))
            .build();
    }

    private Response mockEndpointCheckResponse() {
        String endpoints = "{\"endpoints\":[{\"enabled\":true,\"id\":\"0649c5be323f4792afbc1efdd480847d\"," +
            "\"interface\":\"internal\",\"region\":\"Region1\",\"region_id\":\"Region1\"," +
            "\"service_id\":\"ef6b15e425814dc69d830361baae0e33\",\"url\":\"https://x.x.x.x\"}]}";
        return mockResponseBuilder()
            .body(ResponseBody.create(MediaType.get("application/json"), endpoints))
            .build();
    }

    private Response.Builder mockResponseBuilder() {
        return new Response.Builder().addHeader(KeyStoneConstant.X_SUBJECT_TOKEN_HEADER, "test_token")
            .request(new Request.Builder().url("https://10.9.6.2:443").build())
            .protocol(Protocol.HTTP_2)
            .code(200)
            .message("message success");
    }
}
