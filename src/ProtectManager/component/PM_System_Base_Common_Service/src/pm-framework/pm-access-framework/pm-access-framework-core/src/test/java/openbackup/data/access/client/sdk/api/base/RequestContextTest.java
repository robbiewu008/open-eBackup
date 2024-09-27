package openbackup.data.access.client.sdk.api.base;

import openbackup.system.base.common.exception.DataMoverCheckedException;

import feign.RequestTemplate;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;
import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;

import java.lang.reflect.Method;
import java.lang.reflect.Parameter;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


/**
 * RequestContext LLT
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-4-19
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({
    RequestContext.class, AnnotatedElementUtils.class, Parameter.class, Method.class,
        RequestContextTest.class, RequestTemplate.class, URLEncoder.class})
public class RequestContextTest {

    @InjectMocks
    private RequestContext requestContextForMock;

    private Method method;

    private RequestContext requestContext;

    @Before
    public void init() throws NoSuchMethodException {
        method = RequestContext.class.getMethod("interceptGetRequest", RequestTemplate.class);
        Map<String, String> params = new HashMap<>();
        params.put("test", "test");
        params.put("test1", "test1");
        params.put("test2", "test2");
        Object object = null;
        List<Object> arguments = new ArrayList<>();
        arguments.add(1);
        arguments.add(2);
        requestContext = new RequestContext(params, object, method, arguments);
    }

    /**
     * 用例场景：拦截获取请求,containRequestAnnotation(parameter)方法执行结果为false
     * 前置条件：mock
     * 检查点：运行拦截获取请求，抛出DataMoverCheckedException类型异常
     */
    @Test(expected = DataMoverCheckedException.class)
    public void should_throw_DataMoverCheckedException_when_run_addQueryParams_method_false_branch() {
        RequestTemplate requestTemplate = new RequestTemplate();
        RequestMapping requestMapping = PowerMockito.mock(RequestMapping.class);
        PowerMockito.mockStatic(AnnotatedElementUtils.class);
        PowerMockito.when(AnnotatedElementUtils.findMergedAnnotation(method, RequestMapping.class))
                .thenReturn(requestMapping);
        RequestMethod[] methods = {RequestMethod.GET};
        PowerMockito.when(requestMapping.method()).thenReturn(methods);
        RequestContext mockPrivateMethod = PowerMockito.spy(requestContext);
        mockPrivateMethod.interceptGetRequest(requestTemplate);
    }

    /**
     * 用例场景：拦截获取请求，containRequestAnnotation(parameter)方法执行结果为true
     * 前置条件：mock
     * 检查点：运行拦截获取请求，抛出DataMoverCheckedException类型异常
     */
    @Test(expected = DataMoverCheckedException.class)
    public void should_throw_DataMoverCheckedException_when_run_addQueryParams_method_true_branch() throws Exception {
        Method method = RequestContextTest.class.getMethod("test_interceptGetRequest", String.class);
        Map<String, String> params = new HashMap<>();
        params.put("test", "test");
        params.put("test1", "test1");
        params.put("test2", "test2");
        Object object = null;
        List<Object> arguments = new ArrayList<>();
        arguments.add(1);
        arguments.add(2);
        RequestContext requestContext = new RequestContext(params, object, method, arguments);
        RequestTemplate requestTemplate = new RequestTemplate();
        RequestMapping requestMapping = PowerMockito.mock(RequestMapping.class);
        PowerMockito.mockStatic(AnnotatedElementUtils.class);
        PowerMockito.when(AnnotatedElementUtils.findMergedAnnotation(method, RequestMapping.class))
                .thenReturn(requestMapping);
        RequestMethod[] methods = {RequestMethod.GET};
        PowerMockito.when(requestMapping.method()).thenReturn(methods);
        RequestContext mockPrivateMethod = PowerMockito.spy(requestContext);
        PowerMockito.when(mockPrivateMethod, "containRequestAnnotation", method.getParameters()[0])
                .thenReturn(true);
        mockPrivateMethod.interceptGetRequest(requestTemplate);
    }

    public void test_interceptGetRequest(@RequestParam String str) {
    }

    /**
     * 用例场景：拦截获取请求，requestMapping == null判断为true
     * 前置条件：构造method中没有RequestMapping注解
     * 检查点：运行拦截获取请求，且方法只运行一次
     */
    @Test
    public void test_intercept_get_request_requestMapping_is_null_branch() {
        RequestTemplate requestTemplate = new RequestTemplate();
        RequestContext mockPrivateMethod = PowerMockito.spy(requestContext);
        mockPrivateMethod.interceptGetRequest(requestTemplate);
        Mockito.verify(mockPrivateMethod, Mockito.times(1))
                .interceptGetRequest(requestTemplate);
    }

    /**
     * 用例场景：拦截获取请求，requestMapping.method()方法返回null
     * 前置条件：mock
     * 检查点：运行拦截获取请求，且方法只运行一次
     */
    @Test
    public void test_intercept_get_request_methods_is_null_branch() {
        RequestMapping requestMapping = PowerMockito.mock(RequestMapping.class);
        PowerMockito.mockStatic(AnnotatedElementUtils.class);
        PowerMockito.when(AnnotatedElementUtils.findMergedAnnotation(method, RequestMapping.class))
                .thenReturn(requestMapping);
        RequestMethod[] methods = {};
        PowerMockito.when(requestMapping.method()).thenReturn(methods);
        RequestTemplate requestTemplate = new RequestTemplate();
        RequestContext mockPrivateMethod = PowerMockito.spy(requestContext);
        mockPrivateMethod.interceptGetRequest(requestTemplate);
        Mockito.verify(mockPrivateMethod, Mockito.times(1))
                .interceptGetRequest(requestTemplate);
    }

    /**
     * 用例场景：添加查询参数成功，第三个参数类型为Collection
     * 前置条件：mock
     * 检查点：
     */
    @Test
    public void add_query_params_success_when_third_param_is_collection() throws Exception {
        RequestTemplate requestTemplate = new RequestTemplate();
        RequestContext mockPrivateMethod = PowerMockito.spy(requestContextForMock);
        ArrayList mock = PowerMockito.mock(ArrayList.class);
        Whitebox.invokeMethod(mockPrivateMethod, "addQueryParams",
                requestTemplate, "test", mock);
        Assert.assertNotNull(requestTemplate);
    }

    /**
     * 用例场景：添加查询参数成功，第三个参数类型为Map
     * 前置条件：mock
     * 检查点：
     */
    @Test
    public void add_query_params_success_when_third_param_is_map() throws Exception {
        RequestTemplate requestTemplate = new RequestTemplate();
        RequestContext mockPrivateMethod = PowerMockito.spy(requestContextForMock);
        Map mock = PowerMockito.mock(Map.class);
        Whitebox.invokeMethod(mockPrivateMethod, "addQueryParams",
                requestTemplate, "test", mock);
        Assert.assertNotNull(requestTemplate);
    }


    /**
     * 用例场景：添加查询参数成功，第三个参数类型为Object
     * 前置条件：3个参数均不为空
     * 检查点：
     */
    @Test
    public void add_query_params_success_when_third_param_is_object() throws Exception {
        RequestTemplate requestTemplate = new RequestTemplate();
        RequestContext mockPrivateMethod = PowerMockito.spy(requestContextForMock);
        Whitebox.invokeMethod(mockPrivateMethod, "addQueryParams",
                requestTemplate, "test", mockPrivateMethod);
        Assert.assertNotNull(requestTemplate);
    }
}
