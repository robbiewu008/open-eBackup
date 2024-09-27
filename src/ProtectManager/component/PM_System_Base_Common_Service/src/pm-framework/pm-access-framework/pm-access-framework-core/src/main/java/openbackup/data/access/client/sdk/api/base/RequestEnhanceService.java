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

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.context.annotation.Primary;
import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.stereotype.Component;
import org.springframework.web.bind.annotation.RequestMapping;

import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.BiConsumer;
import java.util.function.Supplier;

/**
 * AnyBackup Enhance Service
 *
 * @author l00272247
 * @since 2020-07-08
 */
@Component
@Primary
public class RequestEnhanceService {
    private ThreadLocal<RequestContext> context = new ThreadLocal<>();

    /**
     * get enhance service context
     *
     * @return context
     */
    public RequestContext getContext() {
        RequestContext value = context.get();
        if (value == null) {
            // Prevents memory leakage when ThreadLocal is not initialized.
            context.remove();
        }
        return value;
    }

    /**
     * run with out context
     *
     * @param supplier supplier
     * @param <T> template type
     * @return result
     */
    public <T> T runWithNoContext(Supplier<T> supplier) {
        RequestContext requestContext = context.get();
        context.set(null);
        try {
            return supplier.get();
        } finally {
            if (requestContext != null) {
                context.set(requestContext);
            } else {
                context.remove();
            }
        }
    }

    /**
     * enhance feign request template
     *
     * @param requestTemplate request template
     * @return context data
     */
    public RequestContext enhance(RequestTemplate requestTemplate) {
        RequestContext requestContext = getContext();
        if (requestContext != null) {
            initParams(requestContext, "query", requestTemplate::query);
            initParams(requestContext, "header", requestTemplate::header);
            Method method = requestContext.getMethod();
            initQueries(requestTemplate, method);
        }
        return requestContext;
    }

    private void initParams(RequestContext requestContext, String type, BiConsumer<String, String[]> consumer) {
        Map<String, String> params = requestContext.getParams();
        String prefix = type + ":";
        for (Map.Entry<String, String> entry : params.entrySet()) {
            String key = entry.getKey();
            if (key.startsWith(prefix)) {
                String name = key.substring(prefix.length());
                String value = entry.getValue();
                consumer.accept(name, new String[] {value});
            }
        }
    }

    /**
     * intercept get request
     *
     * @param requestTemplate request template
     * @return RequestContext
     */
    public RequestContext interceptGetRequest(RequestTemplate requestTemplate) {
        RequestContext requestContext = enhance(requestTemplate);
        if (requestContext != null) {
            requestContext.interceptGetRequest(requestTemplate);
        }
        return requestContext;
    }

    private void initQueries(RequestTemplate requestTemplate, Method method) {
        RequestMapping requestMapping = AnnotatedElementUtils.findMergedAnnotation(method, RequestMapping.class);
        if (requestMapping == null) {
            return;
        }
        String[] params = requestMapping.params();
        for (String param : params) {
            int index = param.indexOf('=');
            if (index != -1) {
                String key = param.substring(0, index);
                String value = param.substring(index + 1);
                requestTemplate.query(key, value);
            } else {
                requestTemplate.query(param);
            }
        }
    }

    /**
     * create proxy
     *
     * @param service service
     * @param params params
     * @param type type
     * @param <T> template type
     * @return proxy service
     */
    public <T> T createProxy(T service, Class<T> type, String... params) {
        return createProxy(service, type, castAsMap(params));
    }

    private <T> T createProxy(T service, Class<T> type, Map<String, String> params) {
        FeignClient feignClient = AnnotatedElementUtils.findMergedAnnotation(service.getClass(), FeignClient.class);
        if (feignClient == null) {
            return service;
        }
        return type.cast(Proxy.newProxyInstance(type.getClassLoader(), new Class[] {type},
            (proxy, method, args) -> invoke(service, params, proxy, method, Arrays.asList(args))));
    }

    /**
     * invoke feign client method
     *
     * @param service service
     * @param params params
     * @param proxy proxy
     * @param method method
     * @param arguments arguments
     * @param <T> template type
     * @return result
     * @throws Throwable IllegalAccessException，InvocationTargetException
     */
    protected <T> Object invoke(T service, Map<String, String> params, Object proxy, Method method,
        List<Object> arguments) throws Throwable {
        RequestContext options = context.get();
        try {
            if (options == null || !options.isReuse()) {
                Map<String, String> map = new HashMap<>(params);
                context.set(new RequestContext(map, proxy, method, arguments));
            }
            return method.invoke(service, arguments.toArray(new Object[0]));
        } finally {
            if (options == null) {
                context.remove();
            } else {
                context.set(options);
            }
        }
    }

    private Map<String, String> castAsMap(String[] params) {
        Map<String, String> map = new HashMap<>();
        if (params == null) {
            return map;
        }
        for (String param : params) {
            int index = param.indexOf(":");
            if (index != -1) {
                String prefix = param.substring(0, index);
                String suffix = param.substring(index + 1);
                map.put(prefix, suffix);
            }
        }
        return map;
    }
}
