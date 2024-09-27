package openbackup.access.framework.resource.service.proxy;

import org.springframework.cglib.proxy.Enhancer;
import org.springframework.cglib.proxy.MethodInterceptor;
import org.springframework.util.ClassUtils;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Proxy Factory
 *
 * @param <T> template type
 * @author l00650874
 * @since 2022-06-07
 */
public class ProxyFactory<T> {
    private static final ConcurrentHashMap<Class<?>, ProxyFactory<?>> CACHES = new ConcurrentHashMap<>();

    private final Class<T> type;

    private ProxyFactory(Class<T> type) {
        this.type = Objects.requireNonNull(type);
    }

    /**
     * get proxy factory by type
     *
     * @param type type
     * @param <T> template type
     * @return proxy factory
     */
    public static <T> ProxyFactory<T> get(Class<T> type) {
        return (ProxyFactory<T>) CACHES.computeIfAbsent(type, ProxyFactory::new);
    }

    /**
     * create proxy object
     *
     * @param items items
     * @return proxy object
     */
    public T create(List<Object> items) {
        List<Object> list = items.stream().filter(type::isInstance).collect(Collectors.toList());
        Enhancer enhancer = new Enhancer();
        enhancer.setSuperclass(type);
        MethodInterceptor interceptor = (object, method, args, proxy) -> intercept(list, method, args);
        enhancer.setCallback(interceptor);
        return type.cast(enhancer.create());
    }

    private Object intercept(List<Object> items, Method method, Object[] args) throws Throwable {
        String name = method.getName();
        boolean isGetter = name.startsWith("get") || name.startsWith("is");
        if (isGetter) {
            return get(items, method, args);
        }
        if (!items.isEmpty()) {
            Object item = items.get(0);
            return method.invoke(item, args);
        } else {
            throw new IllegalArgumentException("items is empty");
        }
    }

    @SuppressWarnings({"rawtypes", "unchecked"})
    private Object get(List<Object> items, Method method, Object[] args)
            throws InvocationTargetException, IllegalAccessException {
        Class<?> returnType = method.getReturnType();
        boolean isPrimitive = ClassUtils.isPrimitiveOrWrapper(returnType);
        String typeName = returnType.getName();
        isPrimitive |=
                typeName.startsWith("java.") && Stream.of(Map.class).noneMatch(e -> e.isAssignableFrom(returnType));
        if (isPrimitive) {
            // 基本数据类型场景，返回首个非空值。
            for (Object item : items) {
                Object value = method.invoke(item, args);
                if (value != null) {
                    return value;
                }
            }
            return null;
        }
        // 复合数据类型，构造返回代理对象。
        ProxyFactory factory = ProxyFactory.get(returnType);
        List<Object> list = new ArrayList<>(items.size());
        for (Object item : items) {
            Object map = method.invoke(item, args);
            if (map != null) {
                list.add(map);
            }
        }
        if (list.isEmpty()) {
            // 所有属性值全部为空，直接返回空。
            return null;
        }
        return factory.create(list);
    }
}
