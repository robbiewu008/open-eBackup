/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.data.access.framework.livemount.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.powermock.api.mockito.PowerMockito.doNothing;
import static org.powermock.api.mockito.PowerMockito.when;

import openbackup.data.access.framework.livemount.controller.policy.PolicyController;
import openbackup.data.access.framework.livemount.controller.policy.request.CreatePolicyRequest;
import openbackup.data.access.framework.livemount.controller.policy.request.UpdatePolicyRequest;
import openbackup.data.access.framework.livemount.controller.policy.response.LiveMountPolicyVo;
import openbackup.data.access.framework.livemount.converter.PolicyDataConverter;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.data.access.framework.livemount.service.PolicyService;
import openbackup.system.base.common.aspect.OperationLogAspect;
import openbackup.system.base.common.aspect.OperationLogService;
import openbackup.system.base.common.aspect.RightsControlInterceptor;
import openbackup.system.base.common.aspect.StringifyConverter;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.common.utils.RightsControl;
import openbackup.system.base.sdk.auth.AuthRestApi;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;

import openbackup.system.base.sdk.copy.model.BasePage;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.springframework.aop.aspectj.annotation.AspectJProxyFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.servlet.handler.DispatcherServletWebRequest;
import org.springframework.web.servlet.mvc.method.annotation.RequestMappingHandlerMapping;

import java.util.ArrayList;

/**
 * test Policy Controller
 *
 */
@RunWith(SpringRunner.class)
@PrepareForTest(PolicyController.class)
@SpringBootTest
@ContextConfiguration(
        classes = {
            PolicyController.class,
            OperationLogAspect.class,
            StringifyConverter.class,
            PolicyService.class,
            PolicyDataConverter.class
        })
public class PolicyControllerTest {
    @InjectMocks
    @Autowired
    private OperationLogAspect operationLogAspect;

    @MockBean
    private TokenVerificationService tokenVerificationService;

    @MockBean
    private RightsControlInterceptor rightsControlInterceptor;

    @MockBean
    private OperationLogService operationLogService;

    @MockBean
    private PolicyService policyService;

    @MockBean
    private RequestMappingHandlerMapping requestMappingHandlerMapping;

    @Autowired
    private PolicyController policyController;

    @Autowired
    private PolicyDataConverter policyDataConverter;

    @MockBean
    private LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    @MockBean
    private AuthRestApi authRestApi;

    @MockBean
    private AuthNativeApi authNativeApi;

    /**
     * before set LiveMountController
     */
    @Before
    public void setLiveMountController() {
        AspectJProxyFactory factory = new AspectJProxyFactory(policyController);
        factory.addAspect(operationLogAspect);
        policyController = factory.getProxy();
        TokenBo.UserBo userBo = TokenBo.UserBo.builder().name("user").id("id").build();
        TokenBo tokenBo = TokenBo.builder().user(userBo).build();
        when(tokenVerificationService.parsingTokenFromRequest()).thenReturn(tokenBo);
        when(rightsControlInterceptor.getSupportedAnnotationType()).thenReturn(RightsControl.class);
        doNothing().when(rightsControlInterceptor).intercept(any(), any(), anyMap(), any());
    }

    /**
     * createPolicy
     */
    @Test
    public void createPolicy() {
        CreatePolicyRequest createPolicyRequest = new CreatePolicyRequest();
        createPolicyRequest.setName("name");
        MockHttpServletRequest mockHttpServletRequest = new MockHttpServletRequest();
        DispatcherServletWebRequest dispatcherServletWebRequest =
                new DispatcherServletWebRequest(mockHttpServletRequest);
        TokenBo token = TokenBo.builder().build();
        mockHttpServletRequest.setAttribute(OperationLogAspect.TOKEN_BO, token);
        RequestContextHolder.setRequestAttributes(dispatcherServletWebRequest);
        policyController.createPolicy(createPolicyRequest);
        Assert.assertNotNull(policyController);
    }

    /**
     * updatePolicy
     */
    @Test
    public void updatePolicy() {
        policyController.updatePolicy("1", new UpdatePolicyRequest());
        Assert.assertNotNull(policyController);
    }

    /**
     * getPolicies
     */
    @Test
    public void getPolicies() {
        BasePage<LiveMountPolicyEntity> policies = policyController.getPolicies(0, 0, "-createtime", new ArrayList<>());
        Assert.assertNull(policies);
    }

    /**
     * getPolicy
     */
    @Test
    public void getPolicy() {
        LiveMountPolicyVo policy = policyController.getPolicy("1");
        Assert.assertNull(policy);
    }

    /**
     * deletePolicy
     */
    @Test
    public void deletePolicy() {
        policyController.deletePolicy("1");
        Assert.assertNotNull(policyController);
    }
}
