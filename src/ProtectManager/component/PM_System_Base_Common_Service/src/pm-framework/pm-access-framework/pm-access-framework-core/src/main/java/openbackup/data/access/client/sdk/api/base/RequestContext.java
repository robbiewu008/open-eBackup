package openbackup.data.access.client.sdk.api.base;

import openbackup.system.base.common.utils.json.JsonUtil;

import feign.RequestTemplate;
import lombok.Getter;
import lombok.Setter;
import lombok.extern.slf4j.Slf4j;

import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;

import java.io.UnsupportedEncodingException;
import java.lang.annotation.Annotation;
import java.lang.reflect.Method;
import java.lang.reflect.Parameter;
import java.net.URLEncoder;
import java.util.Collection;
import java.util.List;
import java.util.Map;

/**
 * Request Context
 *
 * @author l00272247
 * @since 2020/07/23
 */
@Getter
@Slf4j
public class RequestContext {
    private final Map<String, String> params;

    private final Object object;

    private final Method method;

    private final List<Object> arguments;

    @Setter
    private boolean isReuse;

    /**
     * constructor
     *
     * @param params params
     * @param object object
     * @param method method
     * @param arguments arguments
     */
    public RequestContext(Map<String, String> params, Object object, Method method, List<Object> arguments) {
        this.params = params;
        this.object = object;
        this.method = method;
        this.arguments = arguments;
    }

    /**
     * interceptGetRequest
     *
     * @param requestTemplate request template
     */
    public void interceptGetRequest(RequestTemplate requestTemplate) {
        log.debug("send request. method: {}, url: {}.", requestTemplate.method(), requestTemplate.url());
        RequestMapping requestMapping = AnnotatedElementUtils.findMergedAnnotation(method, RequestMapping.class);
        if (requestMapping == null) {
            return;
        }
        RequestMethod[] methods = requestMapping.method();
        if (methods.length != 1 || methods[0] != RequestMethod.GET) {
            return;
        }
        Parameter[] parameters = method.getParameters();
        for (int index = 0; index < parameters.length; index++) {
            Parameter parameter = parameters[index];
            if (!containRequestAnnotation(parameter)) {
                addQueryParams(requestTemplate, arguments.get(index));
            } else {
                RequestParam requestParam = parameter.getAnnotation(RequestParam.class);
                if (requestParam != null && requestParam.value().isEmpty()) {
                    addQueryParams(requestTemplate, arguments.get(index));
                }
            }
        }
    }

    private boolean containRequestAnnotation(Parameter parameter) {
        Annotation[] annotations = parameter.getAnnotations();
        Package pkg = RequestParam.class.getPackage();
        for (Annotation annotation : annotations) {
            Class<?> annotationType = annotation.annotationType();
            if (pkg.equals(annotationType.getPackage())) {
                return true;
            }
        }
        return false;
    }

    private void addQueryParams(RequestTemplate requestTemplate, Object data) {
        if (data == null) {
            return;
        }
        Map<?, ?> map = JsonUtil.cast(data, Map.class);
        map.forEach((key, value) -> addQueryParams(requestTemplate, key.toString(), value));
    }

    private void addQueryParams(RequestTemplate requestTemplate, String name, Object value) {
        if (value == null) {
            requestTemplate.query(name);
        } else if (value instanceof Collection) {
            addQueryParams(requestTemplate, name, (Collection<?>) value);
        } else if (value instanceof Map) {
            addQueryParams(requestTemplate, name, (Map<?, ?>) value);
        } else {
            try {
                // 未做url编码时, value中特殊字符如#会导致请求异常
                requestTemplate.query(name, URLEncoder.encode(value.toString(), "UTF-8"));
            } catch (UnsupportedEncodingException e) {
                log.error("URL encode failed, enc is utf-8.", e);
                requestTemplate.query(name, value.toString());
            }
        }
    }

    private void addQueryParams(RequestTemplate requestTemplate, String name, Collection<?> data) {
        String[] values = data.stream().map(item -> item != null ? item.toString() : "").toArray(String[]::new);
        requestTemplate.query(name, values);
    }

    private void addQueryParams(RequestTemplate requestTemplate, String name, Map<?, ?> data) {
        data.forEach((key, value) -> addQueryParams(requestTemplate, name + "." + key.toString(), value));
    }
}
