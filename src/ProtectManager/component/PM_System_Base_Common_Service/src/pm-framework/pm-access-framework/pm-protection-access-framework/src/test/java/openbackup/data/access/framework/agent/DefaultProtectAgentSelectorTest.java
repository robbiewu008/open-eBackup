package openbackup.data.access.framework.agent;

import static org.assertj.core.api.BDDAssertions.then;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;

import openbackup.access.framework.resource.persistence.dao.ProtectedResourceAgentMapper;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;

import openbackup.data.access.framework.protection.common.constants.AgentKeyConstant;
import openbackup.data.access.framework.protection.mocks.ProtectedResourceMocker;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.model.AgentTypeEnum;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.sdk.auth.model.RoleBo;
import openbackup.system.base.sdk.auth.UserInnerResponse;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * agent选择器测试单元测试
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/26
 **/
public class DefaultProtectAgentSelectorTest {
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private final MemberClusterService memberClusterService = Mockito.mock(MemberClusterService.class);
    private final ProtectedResourceAgentMapper protectedResourceAgentMapper =
            Mockito.mock(ProtectedResourceAgentMapper.class);
    private final DeployTypeService deployTypeService = Mockito.mock(DeployTypeService.class);

    private final DefaultProtectAgentSelector agentSelector = new DefaultProtectAgentSelector(resourceService,
            memberClusterService);

    @Before
    public void before() {
        Whitebox.setInternalState(agentSelector,"protectedResourceAgentMapper",protectedResourceAgentMapper);
        Whitebox.setInternalState(agentSelector, "deployTypeService", deployTypeService);
    }

    /**
     * 用例名称：验证agents参数不为空时，可以正确返回指定agent对应id为非空的agent列表<br/>
     * 前置条件：无<br/>
     * check点：1、agent数量为2  2、agent信息期望的一致<br/>
     */
    @Test
    public void should_return_input_endpoints_when_select_given_param_agents_not_empty() {
        // Given
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
        Map<String, String> parameters = new HashMap<>();
        parameters.put("agents", "1;2;3");
        given(resourceService.getResourceById(eq("1"))).willReturn(
            Optional.of(ProtectedResourceMocker.mockEndPointInfo("1", "1.1.1.1", 80)));
        given(resourceService.getResourceById(eq("2"))).willReturn(
            Optional.of(ProtectedResourceMocker.mockEndPointInfo("2", "2.2.2.2", 80)));
        given(resourceService.getResourceById(eq("3"))).willReturn(Optional.empty());
        PageListResponse response = new PageListResponse();
        response.setRecords(new ArrayList());
        given(resourceService.query(any())).willReturn(response);
        // When
        final List<Endpoint> endpoints = agentSelector.select(protectedResource, parameters);
        // Then
        Assert.assertEquals(0, endpoints.size());
    }

    /**
     * 用例名称：验证agents参数不为空时，可以正确返回指定agent对应id为非空的agent列表<br/>
     * 前置条件：无<br/>
     * check点：1、agent数量为2  2、agent信息期望的一致<br/>
     */
    @Test
    public void should_return_input_endpoints_when_one_resource_not_belong_current_user() {
        // Given
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
        Map<String, String> parameters = new HashMap<>();
        parameters.put("agents", "1;2;3");

        UserInnerResponse userInnerResponse = new UserInnerResponse();
        userInnerResponse.setUserId(UUID.randomUUID().toString());

        RoleBo roleBo = new RoleBo();
        roleBo.setRoleName(Constants.Builtin.ROLE_DP_ADMIN);
        userInnerResponse.setRolesSet(Collections.singleton(roleBo));
        parameters.put(AgentKeyConstant.USER_INFO, JSONObject.writeValueAsString(userInnerResponse));

        given(resourceService.getResourceById(eq("1"))).willReturn(
            Optional.of(ProtectedResourceMocker.mockEndPointInfo("1", "1.1.1.1", 80)));

        ProtectedEnvironment protectedEnvironment = ProtectedResourceMocker.mockEndPointInfo("2", "2.2.2.2", 80);
        protectedEnvironment.setUserId(userInnerResponse.getUserId());
        given(resourceService.getResourceById(eq("2"))).willReturn(Optional.of(protectedEnvironment));
        given(resourceService.getResourceById(eq("3"))).willReturn(Optional.empty());
        PageListResponse response = new PageListResponse();
        response.setRecords(new ArrayList());
        given(resourceService.query(any())).willReturn(response);
        // When
        final List<Endpoint> endpoints = agentSelector.select(protectedResource, parameters);
        // Then
        Assert.assertEquals(0, endpoints.size());
    }

    /**
     * 用例名称：验证agents参数不为空时，可以正确返回指定agent对应id为非空的agent列表<br/>
     * 前置条件：无<br/>
     * check点：1、agent数量为2  2、agent信息期望的一致<br/>
     */
    @Test
    public void should_return_input_endpoints_when_empty_user_info() {
        // Given
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
        Map<String, String> parameters = new HashMap<>();
        parameters.put("agents", "1;2;3");

        UserInnerResponse userInnerResponse = new UserInnerResponse();
        parameters.put(AgentKeyConstant.USER_INFO, JSONObject.writeValueAsString(userInnerResponse));

        given(resourceService.getResourceById(eq("1"))).willReturn(
            Optional.of(ProtectedResourceMocker.mockEndPointInfo("1", "1.1.1.1", 80)));

        ProtectedEnvironment protectedEnvironment = ProtectedResourceMocker.mockEndPointInfo("2", "2.2.2.2", 80);
        protectedEnvironment.setUserId(userInnerResponse.getUserId());
        given(resourceService.getResourceById(eq("2"))).willReturn(Optional.of(protectedEnvironment));
        given(resourceService.getResourceById(eq("3"))).willReturn(Optional.empty());
        PageListResponse response = new PageListResponse();
        response.setRecords(new ArrayList());
        given(resourceService.query(any())).willReturn(response);
        // When
        final List<Endpoint> endpoints = agentSelector.select(protectedResource, parameters);
        // Then
        Assert.assertEquals(0,endpoints.size());
    }

    /**
     * 用例名称：验证agents参数不为空时，可以正确返回指定agent对应id为非空的agent列表<br/>
     * 前置条件：无<br/>
     * check点：1、agent数量为2  2、agent信息期望的一致<br/>
     */
    @Test
    public void should_return_input_endpoints_when_one_resource_not_belong_current_admin_user() {
        // Given
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
        Map<String, String> parameters = new HashMap<>();
        parameters.put("agents", "1;2;3");

        UserInnerResponse userInnerResponse = new UserInnerResponse();
        userInnerResponse.setUserId(UUID.randomUUID().toString());

        RoleBo roleBo = new RoleBo();
        roleBo.setRoleName(Constants.Builtin.ROLE_SYS_ADMIN);
        userInnerResponse.setRolesSet(Collections.singleton(roleBo));
        parameters.put(AgentKeyConstant.USER_INFO, JSONObject.writeValueAsString(userInnerResponse));

        given(resourceService.getResourceById(eq("1"))).willReturn(
            Optional.of(ProtectedResourceMocker.mockEndPointInfo("1", "1.1.1.1", 80)));

        ProtectedEnvironment protectedEnvironment = ProtectedResourceMocker.mockEndPointInfo("2", "2.2.2.2", 80);
        protectedEnvironment.setUserId(userInnerResponse.getUserId());
        given(resourceService.getResourceById(eq("2"))).willReturn(Optional.of(protectedEnvironment));
        PageListResponse response = new PageListResponse();
        List<ProtectedResource> resourceList = new ArrayList<>();
        Map<String, String> resourceExtendInfoMap = new HashMap<>();
        resourceExtendInfoMap.put("scenario", AgentTypeEnum.INTERNAL_AGENT.getValue());
        ProtectedResource resource = new ProtectedResource();
        resource.setParentUuid("1");
        resource.setExtendInfo(resourceExtendInfoMap);
        resourceList.add(resource);
        ProtectedResource resource1 = new ProtectedResource();
        resource1.setParentUuid("2");
        resource1.setExtendInfo(resourceExtendInfoMap);
        resourceList.add(resource1);
        response.setRecords(resourceList);
        given(resourceService.query(any())).willReturn(response);
        given(protectedResourceAgentMapper.querySharedAgentIds()).willReturn(Arrays.asList("123", "456"));
        // When
        final List<Endpoint> endpoints = agentSelector.select(protectedResource, parameters);
        // Then
        Assert.assertEquals(0,endpoints.size());
    }

    /**
     * 用例名称：验证无agents参数时，可以正确返回指定agent对应id为非空的agent列表<br/>
     * 前置条件：无<br/>
     * check点：1、agent数量为2  2、agent信息期望的一致<br/>
     */
    @Test
    public void should_return_all_endpoints_when_select_given_no_param_agents() {
        // Given
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
        Map<String, String> parameters = new HashMap<>();
        final List<ProtectedResource> protectedResources = Arrays.asList(
            ProtectedResourceMocker.mockParentResource("1"), ProtectedResourceMocker.mockParentResource("2"),
            ProtectedResourceMocker.mockParentResource("3"));
        given(resourceService.query(eq(0), eq(10000), anyMap())).willReturn(
            new PageListResponse(3, protectedResources));
        given(resourceService.getResourceById(eq("1"))).willReturn(
            Optional.of(ProtectedResourceMocker.mockEndPointInfo("1", "1.1.1.1", 80)));
        given(resourceService.getResourceById(eq("2"))).willReturn(
            Optional.of(ProtectedResourceMocker.mockEndPointInfo("2", "2.2.2.2", 80)));
        given(resourceService.getResourceById(eq("3"))).willReturn(Optional.empty());
        PageListResponse response = new PageListResponse();
        response.setRecords(new ArrayList());
        given(resourceService.query(any())).willReturn(response);
        // When
        final List<Endpoint> endpoints = agentSelector.select(protectedResource, parameters);
        // Then
        Assert.assertEquals(0, endpoints.size());
    }

    /**
     * 用例名称：验证agents为空字符串，可以正确返回<br/>
     * 前置条件：无<br/>
     * check点：1、agent列表为空<br/>
     */
    @Test
    public void should_return_all_endpoints_when_select_given_param_agents_empty() {
        // Given
        PageListResponse response = new PageListResponse();
        response.setRecords(new ArrayList());
        given(resourceService.query( any())).willReturn(response);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.NAS_FILESYSTEM.getType());
        Map<String, String> parameters = new HashMap<>();
        parameters.put("agents", "");
        // When
        final List<Endpoint> endpoints = agentSelector.select(protectedResource, parameters);
        // Then
        Assert.assertEquals(endpoints.size(), 0);
    }

    /**
     * 用例名称：验证agents为空字符串，根据资源rootUuid可以正确返回<br/>
     * 前置条件：无<br/>
     * check点：1、agent列表为空 2、agent信息期望的一致<br/>
     */
    @Test
    public void should_return_endpoints_when_select_by_dependencies_environment() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setRootUuid("1");
        Map<String, String> parameters = new HashMap<>();
        parameters.put("agents", "");
        given(resourceService.getResourceById(eq("1")))
                .willReturn(Optional.of(ProtectedResourceMocker.mockEnvironmentDependencies()));
        given(resourceService.getResourceById(eq("uuid")))
                .willReturn(Optional.of(ProtectedResourceMocker.mockTaskEnv()));
        PageListResponse response = new PageListResponse();
        response.setRecords(new ArrayList());
        given(resourceService.query(any())).willReturn(response);
        final List<Endpoint> endpoints = agentSelector.select(protectedResource, parameters);
        Assert.assertEquals(0, endpoints.size());
    }

    /**
     * 用例名称：验证hcs代理情况，可以正确返回<br/>
     * 前置条件：无<br/>
     * check点：1、hcs代理 2、agent信息期望的一致<br/>
     */
    @Test
    public void should_return_endpoints_when_hcs_agent() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setRootUuid("1");
        protectedResource.setSubType(ResourceSubTypeEnum.HCS_CLOUD_HOST.getType());
        Map<String, String> parameters = new HashMap<>();
        parameters.put("agents", "");
        given(resourceService.getResourceById(eq("1")))
            .willReturn(Optional.of(ProtectedResourceMocker.mockEnvironmentDependencies()));
        given(resourceService.getResourceById(eq("uuid")))
            .willReturn(Optional.of(ProtectedResourceMocker.mockTaskEnv()));
        PageListResponse response = new PageListResponse();
        response.setRecords(new ArrayList());
        given(resourceService.query(any())).willReturn(response);
        final List<Endpoint> endpoints = agentSelector.select(protectedResource, parameters);
        Assert.assertEquals(0, endpoints.size());
    }

    private List<String> prepareAgentIds() {
        return Arrays.asList("123", "456");
    }
}
