/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.query;

import openbackup.system.base.common.aspect.OperationLogAspect;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.query.SessionService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.web.context.request.RequestAttributes;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletWebRequest;

/**
 * Session Service Test
 *
 * @author l00650874
 * @since 2022-04-01
 */
@RunWith(MockitoJUnitRunner.class)
public class SessionServiceTest {
    /**
     * 用例名称：验证获取当前会话用户信息。<br/>
     * 前置条件：当前用户信息正确。<br/>
     * check点：<br/>
     * 1、无会话场景获取用户信息为空；<br/>
     * 2、内部REST请求场景用户信息为空；<br/>
     * 3、外部REST请求场景用户信息正确。<br/>
     * 4、内部用户切换场景用户信息正确。<br/>
     */
    @Test
    public void test_get_current_user() {
        SessionService sessionService = new SessionService();
        Assert.assertNull(sessionService.getCurrentUser());

        RequestContextHolder.setRequestAttributes(PowerMockito.mock(RequestAttributes.class));
        Assert.assertNull(sessionService.getCurrentUser());

        MockHttpServletRequest internalRequest = new MockHttpServletRequest("get", "/v1/internal/interface");

        RequestContextHolder.setRequestAttributes(new ServletWebRequest(internalRequest));
        Assert.assertNull(sessionService.getCurrentUser());

        MockHttpServletRequest externalRequest = new MockHttpServletRequest("get", "/v1/resources");
        TokenBo.UserBo requestUserBo = TokenBo.UserBo.builder().build();
        TokenBo tokenBo = TokenBo.builder().user(requestUserBo).build();
        externalRequest.setAttribute(OperationLogAspect.TOKEN_BO, tokenBo);
        RequestContextHolder.setRequestAttributes(new ServletWebRequest(externalRequest));
        Assert.assertSame(requestUserBo, sessionService.getCurrentUser());

        TokenBo.UserBo specialUserBo = TokenBo.UserBo.builder().build();
        Assert.assertSame(specialUserBo, sessionService.call(sessionService::getCurrentUser, specialUserBo));
    }

    /**
     * 用例名称：验证mock信息是否正确。<br/>
     * 前置条件：角色参数正确。<br/>
     * check点：mock角色信息正确；<br/>
     */
    @Test
    public void test_mock() {
        SessionService sessionService = new SessionService();
        Assert.assertEquals(1, sessionService.mock("a").getRoles().size());
        Assert.assertEquals("a", sessionService.mock("a").getRoles().get(0).getName());
        Assert.assertEquals(2, sessionService.mock(new String[] {"a", "b"}).getRoles().size());
        Assert.assertEquals("b", sessionService.mock(new String[] {"a", "b"}).getRoles().get(1).getName());
    }
}
