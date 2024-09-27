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
package openbackup.data.access.client.sdk.api.base;

import feign.RequestTemplate;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;
import org.springframework.web.bind.annotation.RequestMapping;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Supplier;

/**
 * RequestEnhanceService LLT
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023/4/20
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({RequestEnhanceService.class, RequestContext.class, RequestTemplate.class})
public class RequestEnhanceServiceTest {

    private RequestEnhanceService mock;

    private Supplier<String> listSupplier;

    private RequestEnhanceService requestEnhanceService;

    private RequestContext requestContext;

    @Before
    public void init() throws NoSuchMethodException {
        Method method = RequestEnhanceServiceTest.class.getMethod("test_for_enhance");
        Map<String, String> params = new HashMap<>();
        params.put("query:test", "test");
        params.put("test1", "test1");
        params.put("test2", "test2");
        Object object = null;
        List<Object> arguments = new ArrayList<>();
        arguments.add(1);
        arguments.add(2);
        requestContext = new RequestContext(params, object, method, arguments);
        ThreadLocal<RequestContext> context = new ThreadLocal<>();
        context.set(requestContext);
        requestEnhanceService = new RequestEnhanceService();
        mock = PowerMockito.spy(requestEnhanceService);
        Whitebox.setInternalState(mock, "context", context);
        listSupplier = new Supplier<String>() {
            @Override
            public String get() {
                return null;
            }
        };
    }

    /**
     * 用例场景：获取请求上下文，获取的值不为空
     * 前置条件：mock
     * 检查点：获取的值不为空
     */
    @Test
    public void get_context_success() {
        RequestContext value = mock.getContext();
        Assert.assertNotNull(value);
    }

    /**
     * 用例场景：获取请求上下文，获取的值为空
     * 前置条件：
     * 检查点：获取的值为空
     */
    @Test
    public void get_context_value_is_null_branch() {
        RequestContext value = requestEnhanceService.getContext();
        Assert.assertNull(value);
    }

    /**
     * 用例场景：以没有上下文的方式运行，requestContext为空
     * 前置条件：
     * 检查点：供应者提供为null
     */
    @Test
    public void should_return_null_if_supplier_supply_is_null() {
        String context = requestEnhanceService.runWithNoContext(listSupplier);
        Assert.assertNull(context);
    }

    /**
     * 用例场景：以没有上下文的方式运行，requestContext不为空
     * 前置条件：
     * 检查点：供应者提供为null
     */
    @Test
    public void should_return_null_if_supplier_supply_is_null_requestContext_not_null_branch() {
        String context = mock.runWithNoContext(listSupplier);
        Assert.assertNull(context);
    }

    /**
     * 用例场景：加强给定RequestTemplate，requestContext不为空
     * 前置条件：mock
     * 检查点：方法返回值不为空
     */
    @Test
    public void do_enhance_success() {
        RequestTemplate requestTemplate = PowerMockito.mock(RequestTemplate.class);
        RequestContext context = mock.enhance(requestTemplate);
        Assert.assertNotNull(context);
    }

    @RequestMapping(params = {"=test", "query:test"})
    public void test_for_enhance() {}

    /**
     * 用例场景：拦截获取请求，requestContext不为空
     * 前置条件：mock
     * 检查点：返回值不为null
     */
    @Test
    public void intercept_get_request_success() {
        RequestTemplate requestTemplate = PowerMockito.mock(RequestTemplate.class);
        RequestContext context = mock.interceptGetRequest(requestTemplate);
        Assert.assertNotNull(context);
    }

    /**
     * 用例场景：创建代理成功
     * 前置条件：mock
     * 检查点：返回值不为null
     */
    @Test
    public void create_proxy_success() {
        RequestTemplate requestTemplate = PowerMockito.mock(RequestTemplate.class);
        RequestTemplate template = mock.createProxy(requestTemplate, RequestTemplate.class, "test:");
        Assert.assertNotNull(template);
    }

    /**
     * 用例场景：调用方法成功
     * 前置条件：mock
     * 检查点：返回值不为null
     */
    @Test
    public void invoke_success() throws Throwable {
        RequestTemplate requestTemplate = PowerMockito.mock(RequestTemplate.class);
        Method method = PowerMockito.mock(Method.class);
        Object invoke = requestEnhanceService.invoke(requestTemplate, new HashMap<>(), "test:",
                method, new ArrayList<>());
        Assert.assertNull(invoke);
    }
}
