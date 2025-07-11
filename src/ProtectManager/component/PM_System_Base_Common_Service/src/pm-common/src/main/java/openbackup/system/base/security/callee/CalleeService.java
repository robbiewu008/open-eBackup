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
