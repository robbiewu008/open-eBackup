/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.security.callee;

import org.springframework.beans.BeansException;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Objects;

/**
 * Callee Service
 *
 * @param <T> template type
 * @author l00272247
 * @since 2021-12-14
 */
public class CalleeService<T> implements Callee, ApplicationContextAware {
    private final Class<T> clazz;
    private final Method method;
    private ApplicationContext applicationContext;

    /**
     * constructor
     *
     * @param clazz clazz
     * @param method method
     */
    public CalleeService(Class<T> clazz, Method method) {
        this.clazz = clazz;
        this.method = method;
    }

    /**
     * call method
     *
     * @param args args
     * @return result
     * @throws InvocationTargetException invocation target exception
     * @throws IllegalAccessException illegal access exception
     */
    @Override
    public Object call(Object... args) throws InvocationTargetException, IllegalAccessException {
        T service = applicationContext.getBean(clazz);
        return method.invoke(service, args);
    }

    /**
     * setter of application context
     *
     * @param applicationContext application context
     * @throws BeansException beans exception
     */
    @Override
    public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
        this.applicationContext = Objects.requireNonNull(applicationContext);
    }
}
