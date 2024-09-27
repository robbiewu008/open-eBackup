package openbackup.system.base.security.callee;

import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.BeanCreationException;
import org.springframework.beans.factory.config.BeanDefinition;
import org.springframework.beans.factory.support.AbstractBeanDefinition;
import org.springframework.beans.factory.support.BeanDefinitionBuilder;
import org.springframework.beans.factory.support.BeanDefinitionRegistry;
import org.springframework.context.annotation.ImportBeanDefinitionRegistrar;
import org.springframework.core.type.AnnotationMetadata;
import org.springframework.util.ClassUtils;

import java.lang.reflect.Method;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Callee Method Register
 *
 * @author l00272247
 * @since 2021-12-14
 */
@Slf4j
public class CalleeMethodRegister implements ImportBeanDefinitionRegistrar {
    private final CalleeMethodScanner scanner = new CalleeMethodScanner();

    @Override
    public void registerBeanDefinitions(AnnotationMetadata importingClassMetadata, BeanDefinitionRegistry registry) {
        Map<String, Object> attributes =
            importingClassMetadata.getAnnotationAttributes(CalleeMethodScan.class.getName());
        if (attributes == null) {
            return;
        }
        Object value = attributes.get("value");
        if (!(value instanceof String[])) {
            return;
        }
        List<String> basePackages = new ArrayList<>(Arrays.asList((String[]) value));
        if (basePackages.isEmpty()) {
            String enclosingClassName = Objects.requireNonNull(importingClassMetadata.getClassName());
            basePackages.add(ClassUtils.getPackageName(enclosingClassName));
        }
        for (String basePackage : basePackages) {
            registerRestServiceProxyBeanDefinitions(registry, basePackage);
        }
    }

    private void registerRestServiceProxyBeanDefinitions(BeanDefinitionRegistry registry, String basePackage) {
        Set<BeanDefinition> beanDefinitions = scanner.findCandidateComponents(basePackage);
        Set<String> apiClassNames =
            beanDefinitions.stream().map(BeanDefinition::getBeanClassName).collect(Collectors.toSet());
        apiClassNames.forEach(apiClassName -> registerRestServiceProxyFactoryBean(apiClassName, registry));
    }

    private void registerRestServiceProxyFactoryBean(String apiClassName, BeanDefinitionRegistry registry) {
        constructRestServiceProxyFactoryBean(apiClassName).forEach(registry::registerBeanDefinition);
    }

    private Map<String, BeanDefinition> constructRestServiceProxyFactoryBean(String apiClassName) {
        Class<?> clazz;
        try {
            clazz = Class.forName(apiClassName);
        } catch (ClassNotFoundException e) {
            throw new BeanCreationException("class not found", e);
        }
        CalleeMethods calleeMethods = clazz.getAnnotation(CalleeMethods.class);
        CalleeMethod[] calleeMethodList = clazz.getAnnotationsByType(CalleeMethod.class);
        return Stream
            .concat(
                Stream.of(calleeMethodList)
                    .map(calleeMethod -> getMethodByClassCalleeConfig(clazz, calleeMethods, calleeMethod)),
                Stream.of(clazz.getMethods())
                    .map(calleeMethod -> getMethodByMethodCalleeConfig(clazz, calleeMethods, calleeMethod)))
            .filter(Objects::nonNull)
            .collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue));
    }

    private Map.Entry<String, AbstractBeanDefinition> getMethodByClassCalleeConfig(Class<?> clazz,
        CalleeMethods calleeMethods, CalleeMethod calleeMethod) {
        String name = calleeMethod.name();
        Class<?>[] args = calleeMethod.args();
        boolean isDefault = Arrays.deepEquals(args, new Class<?>[] {CalleeMethod.class});
        Method method;
        if (isDefault) {
            method = getMethodByName(clazz, name);
        } else {
            try {
                method = clazz.getMethod(name, args);
            } catch (NoSuchMethodException e) {
                throw new LegoCheckedException("not found callee method by name: " + name, e);
            }
        }
        String fullname = buildFullname(clazz, calleeMethods, name);
        Map.Entry<String, AbstractBeanDefinition> entry =
            new AbstractMap.SimpleEntry<>(fullname, createCalleeBean(clazz, method));
        log.info("found callee method: {}", fullname);
        return entry;
    }

    private String buildFullname(Class<?> clazz, CalleeMethods calleeMethods, String name) {
        String basename;
        if (calleeMethods.name().isEmpty()) {
            basename = clazz.getSimpleName();
        } else {
            basename = calleeMethods.name();
        }
        return (basename + "_" + name).replaceAll("([^A-Z])([A-Z])", "$1_$2").toLowerCase(Locale.ENGLISH);
    }

    private Method getMethodByName(Class<?> clazz, String name) {
        List<Method> methods =
            Stream.of(clazz.getMethods()).filter(method -> method.getName().equals(name)).collect(Collectors.toList());
        if (methods.isEmpty()) {
            throw new LegoCheckedException("not found method by name: " + name);
        }
        if (methods.size() > 1) {
            throw new LegoCheckedException("found multiple method by name: " + name);
        }
        return methods.get(0);
    }

    private Map.Entry<String, AbstractBeanDefinition> getMethodByMethodCalleeConfig(Class<?> clazz,
        CalleeMethods calleeMethods, Method method) {
        CalleeMethod calleeMethod = method.getAnnotation(CalleeMethod.class);
        if (calleeMethod == null) {
            return null;
        }
        String name = calleeMethod.name().isEmpty() ? method.getName() : calleeMethod.name();
        String fullname = buildFullname(clazz, calleeMethods, name);
        return new AbstractMap.SimpleEntry<>(fullname, createCalleeBean(clazz, method));
    }

    private AbstractBeanDefinition createCalleeBean(Class<?> clazz, Method method) {
        BeanDefinitionBuilder beanDefinitionBuilder =
            BeanDefinitionBuilder.genericBeanDefinition(Callee.class, () -> createCallee(clazz, method));
        return beanDefinitionBuilder.getBeanDefinition();
    }

    private <T> Callee createCallee(Class<T> clazz, Method method) {
        return new CalleeService<>(clazz, method);
    }
}
