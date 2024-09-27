package openbackup.obs.plugin.service.impl;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;

import openbackup.access.framework.resource.service.AgentBusinessService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.obs.plugin.provider.ObjectSetProviderTest;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Answers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author w00607005
 * @since 2023-11-20
 */
@RunWith(PowerMockRunner.class)
@PowerMockIgnore({"javax.management.*", "jdk.internal.reflect.*"})
public class ObjectStorageAgentServiceImplTest {
    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private AgentUnifiedService agentService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ResourceService resourceService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private SessionService sessionService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private AgentBusinessService agentBusinessService;;

    @InjectMocks
    private ObjectStorageAgentServiceImpl objectStorageAgentServiceImpl;

    /**
     * 用例名称：校验连通性成功
     * 前置条件：无
     * 检查点：无报错
     */
    @Test
    public void test_check_connection_success() {
        // setup
        PowerMockito.when(resourceService.check(any(ProtectedEnvironment.class))).thenReturn(new ActionResult[] {});

        ProtectedEnvironment environment = new ProtectedEnvironment();

        // run the test
        ActionResult[] result = objectStorageAgentServiceImpl.checkConnection(environment);

        // verify the results
        Assert.assertEquals(0, result.length);
    }

    @Test
    public void test_getDetail_success() {
        ProtectedEnvironment environment = ObjectSetProviderTest.mockEnvironment();
        ProtectedResource protectedResource = BeanTools.copy(environment, ProtectedResource::new);
        protectedResource.setType(ResourceTypeEnum.STORAGE.getType());
        protectedResource.setSubType(ResourceSubTypeEnum.OBJECT_STORAGE.getType());
        protectedResource.setExtendInfoByKey("pageNo", "1");
        protectedResource.setExtendInfoByKey("pageSize", "20");
        String agents = "56c896fe-fb62-4498-8508-1b37ba5ad11c";
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setEndpoint("10.160.170.10");
        agent.setPort(5985);
        agent.setUuid(agents);
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner(eq(agents))).thenReturn(Optional.of(agent));

        PageListResponse<ProtectedResource> objectPageListResponse = new PageListResponse<>();
        objectPageListResponse.setTotalCount(1);
        ProtectedEnvironment protectedResource1 = new ProtectedEnvironment();
        protectedResource1.setName("bucket");
        objectPageListResponse.setRecords(Collections.singletonList(protectedResource1));

        PowerMockito.when(agentService.getDetailPageList(anyString(), anyString(), anyInt(), any()))
            .thenReturn(objectPageListResponse);
        PageListResponse<ProtectedResource> detail = objectStorageAgentServiceImpl.getDetail(environment,
            protectedResource, agents, ResourceSubTypeEnum.OBJECT_STORAGE);
        Assert.assertNotNull(detail);
    }

    @Test
    public void test_getDetail_throw_exception_when_invoke_agent_detail_fail() {
        ProtectedEnvironment environment = ObjectSetProviderTest.mockEnvironment();
        ProtectedResource protectedResource = BeanTools.copy(environment, ProtectedResource::new);
        protectedResource.setType(ResourceTypeEnum.STORAGE.getType());
        protectedResource.setSubType(ResourceSubTypeEnum.OBJECT_STORAGE.getType());
        protectedResource.setExtendInfoByKey("pageNo", "1");
        protectedResource.setExtendInfoByKey("pageSize", "20");
        String agents = "56c896fe-fb62-4498-8508-1b37ba5ad11c";
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setEndpoint("10.160.170.10");
        agent.setPort(5985);
        agent.setUuid(agents);
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner(eq(agents))).thenReturn(Optional.of(agent));

        PageListResponse<ProtectedResource> objectPageListResponse = new PageListResponse<>();
        objectPageListResponse.setTotalCount(1);
        ProtectedEnvironment protectedResource1 = new ProtectedEnvironment();
        protectedResource1.setName("bucket");
        objectPageListResponse.setRecords(Collections.singletonList(protectedResource1));

        PowerMockito.when(agentService.getDetailPageList(anyString(), anyString(), anyInt(), any()))
            .thenThrow((new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "test")));
        LegoCheckedException exception =
            Assert.assertThrows(LegoCheckedException.class, () -> objectStorageAgentServiceImpl.getDetail(environment,
                protectedResource, agents, ResourceSubTypeEnum.OBJECT_STORAGE));
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, exception.getErrorCode());
    }

    @Test
    public void test_queryBuiltInAgents_success() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap(), anyString()))
            .thenReturn(new PageListResponse<ProtectedResource>());
        PowerMockito.when(sessionService.call(any(), anyString()))
            .thenReturn(new PageListResponse<ProtectedResource>(0, new ArrayList<>()));
        List<ProtectedResource> protectedResources = objectStorageAgentServiceImpl.queryBuiltInAgents();
        Assert.assertNotNull(protectedResources);
    }

    @Test
    public void test_queryBuiltInAgents_fail() {
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap(), anyString()))
            .thenReturn(new PageListResponse<>(1, new ArrayList<>()));
        PowerMockito.when(sessionService.call(any(), anyString()))
            .thenReturn((new PageListResponse<ProtectedResource>(0, new ArrayList<>())));
        List<ProtectedResource> protectedResources = objectStorageAgentServiceImpl.queryBuiltInAgents();
        Assert.assertEquals(0, protectedResources.size());
    }

    @Test
    public void test_queryAgents_success() {
        ArrayList<String> ids = new ArrayList<>();
        ids.add("11");
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource resource = new ProtectedResource();
        resource.setName("11");
        resource.setType("ObjectStorage");
        resources.add(resource);
        PageListResponse<ProtectedResource> protectedResourcePageListResponse = new PageListResponse<>(2, resources);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap(), anyString()))
            .thenReturn(protectedResourcePageListResponse);
        PowerMockito.when(sessionService.call(any(), anyString())).thenReturn((protectedResourcePageListResponse));
        List<ProtectedResource> protectedResources = objectStorageAgentServiceImpl.queryAgents(ids);
        Assert.assertEquals(1, protectedResources.size());
    }
}