/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.controller;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.controller.ProtectedEnvironmentAccessController;
import openbackup.access.framework.resource.dto.BaseProtectedEnvironmentDto;
import openbackup.access.framework.resource.dto.ProtectedEnvironmentDto;
import openbackup.access.framework.resource.service.lock.ResourceDistributedLockService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.aspect.verifier.CommonOwnershipVerifier;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.common.model.UuidObject;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * ProtectedEnvironmentAccessController测试
 *
 * @author h30027154
 * @since 2022-07-18
 * @version [OceanProtect X8000 1.2.1]
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({TokenBo.class})
public class ProtectedEnvironmentAccessControllerTest {

    private final ProtectedEnvironmentService protectedEnvironmentService = Mockito.mock(
            ProtectedEnvironmentService.class, Mockito.RETURNS_DEEP_STUBS);

    private final SessionService sessionService = Mockito.mock(SessionService.class);

    private final CommonOwnershipVerifier resourceOwnershipVerifier = Mockito.mock(CommonOwnershipVerifier.class);

    private final ResourceDistributedLockService distributedLockService =
        Mockito.mock(ResourceDistributedLockService.class, Mockito.RETURNS_DEEP_STUBS);

    private final ProviderManager providerManager = Mockito.mock(ProviderManager.class);

    @Captor
    ArgumentCaptor<List<String>> uuidListCaptor;

    /**
     * 用例名称：验证资源注册接口是否能够正常调用。<br/>
     * 前置条件：无。<br/>
     * check点：正常运行。<br/>
     */
    @Test
    public void register_environment() {
        Mockito.when(protectedEnvironmentService.register(any())).thenReturn("11");

        UuidObject uuidObject = createProtectedEnvironmentAccessController().registerProtectedEnvironment(
                new ProtectedEnvironmentDto());
        Assert.assertNotNull(uuidObject);
        Mockito.verify(distributedLockService, Mockito.times(1))
            .tryLockAndRun(Mockito.any(), Mockito.any(), Mockito.any());
    }

    /**
     * 用例名称：验证资源连通性接口是否能够正常调用。<br/>
     * 前置条件：无。<br/>
     * check点：连通性正常返回结果。<br/>
     */
    @Test
    public void check_environment_connection() {
        Mockito.when(protectedEnvironmentService.checkProtectedEnvironment(any()))
                .thenReturn(new ActionResult[1]);
        ActionResult[] actionResults = createProtectedEnvironmentAccessController().checkEnvironmentConnectivity(
                new ProtectedEnvironmentDto());
        Assert.assertEquals(actionResults.length, 1);
    }

    private TokenBo getMockedTokenBo(TokenBo.RoleBo roleBo) {
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setName("admin");
        userBo.setId("3434456567");
        userBo.setRoles(Collections.singletonList(roleBo));
        long exp = System.currentTimeMillis();
        long created = System.currentTimeMillis();
        TokenBo tokenBo = TokenBo.builder().user(userBo).exp(exp).created(created).build();
        return tokenBo;
    }

    private List<ProtectedResource> getListOfResource(String... uuids) {
        List<String> uuidsList = Arrays.asList(uuids);
        return uuidsList.stream().map(uuid -> {
            ProtectedResource resource = new ProtectedResource();
            resource.setUuid(uuid);
            return resource;
        }).collect(Collectors.toList());
    }

    private Map<String, List<ProtectedResource>> getMockedDependency() {
        HashMap<String, List<ProtectedResource>> dependencyRet = new HashMap<>();
        dependencyRet.put("a", getListOfResource("1", "2"));
        dependencyRet.put("b", getListOfResource("3", "4"));
        dependencyRet.put("c", getListOfResource(""));

        HashMap<String, List<ProtectedResource>> dependencyUuidA2 = new HashMap<>();
        dependencyUuidA2.put("2", getListOfResource("5", "6"));
        dependencyRet.get("a").get(1).setDependencies(dependencyUuidA2);

        HashMap<String, List<ProtectedResource>> dependencyC = new HashMap<>();
        dependencyC.put("", getListOfResource("7"));

        dependencyRet.get("c").get(0).setDependencies(dependencyC);

        return dependencyRet;
    }

    /**
     * 用例名称：验证资源连通性接口是否能够正常调用。<br/>
     * 前置条件：resource中dependency中有其他用户的资源。 调用者非SYSADMIN管理员<br/>
     * check点：resourceOwnershipVerifier.verify函数中接收到的uuid list和 dependency中的uuid一致。<br/>
     */
    @Test
    public void should_verify_right_nums_if_environment_has_some_dependency_when_check_environment_connection() {
        // given
        ProtectedEnvironmentDto protectedEnvironmentDto = new ProtectedEnvironmentDto();
        protectedEnvironmentDto.setDependencies(getMockedDependency());

        TokenBo.RoleBo dpRole = TokenBo.RoleBo.builder().name(Constants.Builtin.ROLE_DP_ADMIN).build();
        TokenBo tokenBo = getMockedTokenBo(dpRole);
        PowerMockito.mockStatic(TokenBo.class);
        PowerMockito.when(TokenBo.get()).thenReturn(tokenBo);

        ActionResult[] actionResults = createProtectedEnvironmentAccessController().checkEnvironmentConnectivity(
                protectedEnvironmentDto);
        Mockito.verify(resourceOwnershipVerifier).verify(any(), uuidListCaptor.capture());
        List<String> uuidList = uuidListCaptor.getValue();
        Assert.assertEquals(uuidList.size(), 7);
    }

    /**
     * 用例名称：验证资源连通性接口是否能够正常调用。<br/>
     * 前置条件：用户角色为SYSADMIN<br/>
     * check点：底层不调用resourceOwnerShipVerifier。<br/>
     */
    @Test
    public void should_do_not_verify_if_environment_has_some_dependency_when_check_environment_connection() {
        // given
        ProtectedEnvironmentDto protectedEnvironmentDto = new ProtectedEnvironmentDto();
        protectedEnvironmentDto.setDependencies(getMockedDependency());

        TokenBo.RoleBo dpRole = TokenBo.RoleBo.builder().name(Constants.Builtin.ROLE_SYS_ADMIN).build();
        TokenBo tokenBo = getMockedTokenBo(dpRole);
        PowerMockito.mockStatic(TokenBo.class);
        PowerMockito.when(TokenBo.get()).thenReturn(tokenBo);

        ActionResult[] actionResults = createProtectedEnvironmentAccessController().checkEnvironmentConnectivity(
                protectedEnvironmentDto);
        Mockito.verify(resourceOwnershipVerifier, Mockito.never()).verify(any(), any());
    }

    /**
     * 用例名称：验证资源更新接口是否能够正常调用。<br/>
     * 前置条件：无。<br/>
     * check点：auth extend是否正常合并。<br/>
     */
    @Test
    public void test_update_environment_param_should_be_set_success() {
        BaseProtectedEnvironmentDto baseProtectedEnvironmentDto = new BaseProtectedEnvironmentDto();
        Authentication inputAuth = new Authentication();
        HashMap<String, String> inputExtendMap = new HashMap<>();
        inputExtendMap.put("a", "va1");
        inputExtendMap.put("c", "vc1");
        inputAuth.setExtendInfo(inputExtendMap);
        baseProtectedEnvironmentDto.setAuth(inputAuth);
        baseProtectedEnvironmentDto.setExtendInfo(new HashMap<>());

        ProtectedEnvironment environment = new ProtectedEnvironment();
        Authentication envAuth = new Authentication();
        HashMap<String, String> extendMap = new HashMap<>();
        extendMap.put("a", "va2");
        extendMap.put("b", "vb2");
        envAuth.setExtendInfo(extendMap);
        environment.setAuth(envAuth);
        Mockito.when(protectedEnvironmentService.getEnvironmentById(any())).thenReturn(environment);

        createProtectedEnvironmentAccessController().updateProtectedEnvironment(null, baseProtectedEnvironmentDto);

        Assert.assertEquals(environment.getAuth().getExtendInfo().get("a"), "va1");
        Assert.assertEquals(environment.getAuth().getExtendInfo().get("b"), "vb2");
        Assert.assertEquals(environment.getAuth().getExtendInfo().get("c"), "vc1");
    }

    /**
     * 用例名称：验证资源浏览接口是否能够正常调用。<br/>
     * 前置条件：无。<br/>
     * check点：正常运行。<br/>
     */
    @Test
    public void browse_environment() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        ArrayList<ProtectedResource> resources = new ArrayList<>();
        resources.add(new ProtectedResource());
        response.setRecords(resources);
        Mockito.when(protectedEnvironmentService.browse(any())).thenReturn(response);
        createProtectedEnvironmentAccessController().browseEnvironmentResource(
                new BrowseEnvironmentResourceConditions());
        Mockito.verify(protectedEnvironmentService, Mockito.times(1)).browse(any());
    }

    private ProtectedEnvironmentAccessController createProtectedEnvironmentAccessController() {
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId("userId");
        Mockito.when(sessionService.getCurrentUser()).thenReturn(userBo);
        return new ProtectedEnvironmentAccessController(protectedEnvironmentService, sessionService,
                resourceOwnershipVerifier, distributedLockService, providerManager);
    }
}