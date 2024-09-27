/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.log;

import openbackup.system.base.common.annotation.OperationContext;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.log.utils.TypeUtil;
import openbackup.system.base.common.utils.ExprUtil;
import openbackup.system.base.common.utils.StringUtil;

import org.springframework.core.MethodParameter;

import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.GenericArrayType;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

/**
 * data load config
 *
 * @author l00272247
 * @since 2019-11-05
 */
public class OperationContextConfig implements Comparable<OperationContextConfig> {
    private OperationContextLoader defaultOperationContextLoader;

    private final Type expectType;

    private final Method method;

    private OperationContext annotation;

    private final MethodParameter methodParameter;

    private OperationContextConfig dependency;

    private boolean isInitialized;

    private final boolean isArrayRelated;

    private Type relatedType;

    /**
     * constructor
     *
     * @param method method
     * @param annotation data load annotation
     * @param methodParameter method parameter
     */
    public OperationContextConfig(Method method, OperationContext annotation, MethodParameter methodParameter) {
        this.method = method;
        this.methodParameter = methodParameter;
        Type type;
        if (annotation != null) {
            this.annotation = annotation;
            checkOperationContextTypeField();
            checkOperationContextLoaderType();
            if (methodParameter == null) {
                // OperationContext配置在方法上的场景
                String name = annotation.name();
                if ("".equals(name)) {
                    throw new LegoCheckedException("name field missing: " + annotation);
                }
                type = Object.class;
            } else {
                // OperationContext配置在参数上的场景
                checkParameterType();
                type = methodParameter.getGenericParameterType();
            }
        } else {
            type = Object.class;
            isInitialized = true;
        }
        expectType = type;
        isArrayRelated = TypeUtil.isArrayRelatedType(type);
        if (isArrayRelated) {
            relatedType = TypeUtil.getArrayElementType(type);
        } else {
            relatedType = type;
        }
        if (annotation != null) {
            if (!OperationContext.class.equals(annotation.type())) {
                // 通过注解配置type类型的场景
                if (!TypeUtil.isAssignableFrom(relatedType, annotation.type())) {
                    // 如果注解配置的type类型不满足参数类型要求，则报错退出
                    throw new LegoCheckedException("type field invalid: " + annotation);
                } else {
                    relatedType = annotation.type();
                }
            }
        }
    }

    private void checkOperationContextTypeField() {
        Class<?> type = annotation.type();
        if (type != OperationContext.class) {
            if (type.isEnum()) {
                throw error("type of OperationContext is not support enum.", null);
            }
            if (type.isInterface()) {
                throw error("type of OperationContext is not support interface.", null);
            }
        }
        if (Collection.class.isAssignableFrom(type)) {
            throw error("type of OperationContext is not support collection.", null);
        }
        if (Map.class.isAssignableFrom(type)) {
            throw error("type of OperationContext is not support map.", null);
        }
    }

    private LegoCheckedException error(String template, Object[] args) {
        String message = String.format(Locale.ENGLISH, template, args) + " " + this;
        return new LegoCheckedException(message);
    }

    public Method getMethod() {
        return method;
    }

    /**
     * toString
     *
     * @return String
     */
    @Override
    public String toString() {
        String result = "method: " + method;
        if (annotation != null) {
            result = result + " annotation: " + annotation;
        }
        return result;
    }

    private void checkOperationContextLoaderType() {
        Class<? extends OperationContextLoader> loaderType = annotation.loader();
        if (!OperationContextLoader.class.equals(loaderType) && loaderType.isInterface()) {
            throw error("loader type should be a implement class of OperationContextLoader.", null);
        }
    }

    private void checkParameterType() {
        Type type = methodParameter.getGenericParameterType();
        if (type instanceof ParameterizedType) {
            if (!TypeUtil.isAssignableFrom(Collection.class, ((ParameterizedType) type).getOwnerType())) {
                throw error(
                        "type of parameter %s is not supported.", new Object[] {methodParameter.getParameterIndex()});
            }
        }
    }

    public OperationContext getAnnotation() {
        return annotation;
    }

    /**
     * get context name, return null if name is empty
     *
     * @return context name
     */
    public String getName() {
        if (annotation == null) {
            return null;
        }
        String name = annotation.name();
        return !"".equals(name) ? name : null;
    }

    /**
     * match expression prefix
     *
     * @param name name
     * @return result
     */
    public boolean matches(String name) {
        if (name == null) {
            return false;
        }
        if (methodParameter != null) {
            if (name.equals(Integer.toString(methodParameter.getParameterIndex()))) {
                return true;
            }
        }
        if (annotation != null && !"".equals(annotation.name())) {
            return name.equals(annotation.name());
        }
        return false;
    }

    /**
     * initialize operation context config context
     *
     * @param operationContextConfigContext operation context config context
     */
    public void initialize(List<OperationContextConfig> operationContextConfigContext) {
        if (isInitialized) {
            return;
        }
        isInitialized = true;
        try {
            String prefix = StringUtil.splitrim(annotation.value(), "[?\\.]").get(0);
            OperationContextConfig config =
                    operationContextConfigContext.stream()
                            .filter(item -> item.matches(prefix))
                            .findFirst()
                            .orElseThrow(() -> error("dependency missing: " + prefix, null));
            dependency = config;
            config.initialize(operationContextConfigContext);
            if (config.dependOn(this)) {
                throw error("circle dependence with " + config, null);
            }
        } catch (LegoCheckedException legoCheckedException) {
            dependency = null;
            isInitialized = false;
            throw legoCheckedException;
        }
    }

    /**
     * detect dependency relation
     *
     * @param config config
     * @return result
     */
    public boolean dependOn(OperationContextConfig config) {
        for (OperationContextConfig depend = dependency; depend != null; depend = depend.dependency) {
            if (Objects.equals(depend, config)) {
                return true;
            }
        }
        return false;
    }

    /**
     * load data
     *
     * @param operationContextLoaders loaders
     * @param paramContext param context
     * @param arguments arguments
     * @return result
     */
    public Object load(
            List<OperationContextLoader> operationContextLoaders,
            Map<String, Object> paramContext,
            List<Object> arguments) {
        if (methodParameter != null && annotation == null) {
            return arguments.get(methodParameter.getParameterIndex());
        }
        Object data = adapt(load(operationContextLoaders, paramContext));
        String name = annotation.name().trim();
        if (!"".equals(name)) {
            paramContext.put(name, data);
        }
        if (methodParameter != null) {
            arguments.add(methodParameter.getParameterIndex(), data);
        }
        return data;
    }

    private Object load(List<OperationContextLoader> operationContextLoaders, Map<String, Object> paramContext) {
        Object value = ExprUtil.eval(paramContext, annotation.value());
        Class<? extends OperationContextLoader> loaderType = annotation.loader();
        OperationContextLoader operationContextLoader;
        if (!OperationContextLoader.class.equals(loaderType)) {
            operationContextLoader =
                    operationContextLoaders.stream()
                            .filter(annotation.loader()::isInstance)
                            .findFirst()
                            .orElseThrow(() -> new LegoCheckedException("not found loader: " + loaderType.getName()));
        } else if (!OperationContext.class.equals(annotation.type())) {
            operationContextLoader = defaultOperationContextLoader;
        } else {
            return value;
        }
        return operationContextLoader.load(this, value);
    }

    private Object adapt(Object data) {
        if (data == null) {
            return null;
        }

        Object result;
        if (isArrayRelated) {
            result = convertAsArrayObject(data);
        } else if (methodParameter != null && TypeUtil.isArrayObject(data)) {
            result = getUniqueItem(data);
        } else {
            result = data;
        }
        return result;
    }

    private Object getUniqueItem(Object data) {
        Object result;
        Collection<?> collection = TypeUtil.castObjectAsCollection(data);
        if (collection == null || collection.isEmpty()) {
            result = null;
        } else if (collection.size() == 1) {
            result = new ArrayList<Object>(collection).get(0);
        } else {
            throw new LegoCheckedException("result is not unique: " + annotation);
        }
        return result;
    }

    private Object convertAsArrayObject(Object data) {
        Object result;
        Collection<?> collection = TypeUtil.castObjectAsCollection(data);
        if (collection != null) {
            collection.forEach(this::validateDataType);
        }
        if (expectType instanceof ParameterizedType) {
            result = convertAsExpectCollection((ParameterizedType) expectType, collection);
        } else {
            result = convertAsExpectArray(collection);
        }
        return result;
    }

    private void validateDataType(Object data) {
        if (data != null) {
            if (!TypeUtil.isAssignableFrom(relatedType, data.getClass())) {
                throw new LegoCheckedException("data type is incorrect. expect: " + relatedType);
            }
        }
    }

    private Object convertAsExpectArray(Collection<?> collection) {
        if (collection == null) {
            return null;
        }
        if (!(relatedType instanceof GenericArrayType)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Type is not instance of GenericArrayType");
        }
        final Type genericComponentType = ((GenericArrayType) relatedType).getGenericComponentType();
        if (!(genericComponentType instanceof Class<?>)) {
            throw new LegoCheckedException(
                    CommonErrorCode.ILLEGAL_PARAM, "genericComponentType is not instance of Class<?>");
        }
        return Array.newInstance((Class<?>) genericComponentType, collection.size());
    }

    private Object convertAsExpectCollection(ParameterizedType parameterizedType, Collection<?> collection) {
        final Type ownerType = parameterizedType.getOwnerType();
        if (!(ownerType instanceof Class<?>)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Type is not instance of Class<?>");
        }
        Class<?> ownerClazz = (Class<?>) ownerType;
        if (ownerClazz == Collection.class) {
            return collection;
        }
        Class<? extends Collection> implement;
        if (ownerClazz.isInterface()) {
            Map<Class<? extends Collection>, Class<? extends Collection>> typeMappings = new HashMap<>();
            typeMappings.put(List.class, ArrayList.class);
            typeMappings.put(Set.class, HashSet.class);
            implement = typeMappings.get(ownerClazz);
            if (implement == null) {
                throw new LegoCheckedException("not supported: " + ownerClazz);
            }
        } else {
            Class<? extends Collection> type = (Class<? extends Collection>) ownerClazz;
            implement = type;
        }
        try {
            Constructor<? extends Collection> constructor = implement.getConstructor(Collection.class);
            return constructor.newInstance(collection);
        } catch (NoSuchMethodException
                | InstantiationException
                | IllegalAccessException
                | InvocationTargetException e) {
            throw new LegoCheckedException("type cast failed", e);
        }
    }

    /**
     * compareTo
     *
     * @param obj obj
     * @return int
     */
    @Override
    public int compareTo(OperationContextConfig obj) {
        if (obj.dependOn(this)) {
            return -1;
        }
        if (this.dependOn(obj)) {
            return 1;
        }
        return 0;
    }
}
