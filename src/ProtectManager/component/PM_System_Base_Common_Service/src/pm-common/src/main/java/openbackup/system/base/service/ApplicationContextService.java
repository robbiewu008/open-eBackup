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
package openbackup.system.base.service;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.context.annotation.AnnotationConfigApplicationContext;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * Application Context Service
 *
 * @author l00272247
 * @since 2021-02-27
 */
@Component
public class ApplicationContextService {
    @Autowired
    private ApplicationContext applicationContext;

    /**
     * autowired
     *
     * @param bean bean
     * @param <T> template type
     * @param dependencies dependencies
     * @return autowired bean
     */
    public <T> T autowired(T bean, List<Object> dependencies) {
        AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext();
        context.setParent(applicationContext);
        if (dependencies != null) {
            for (Object dependency : dependencies) {
                registerBean(context, dependency);
            }
        }
        registerBean(context, bean);
        context.refresh();
        @SuppressWarnings("unchecked")
        T result = (T) context.getBean(bean.getClass());
        context.close();
        return result;
    }

    private <T> void registerBean(AnnotationConfigApplicationContext context, T bean) {
        @SuppressWarnings("unchecked")
        Class<T> clazz = (Class<T>) bean.getClass();
        context.registerBean(clazz, () -> bean);
    }

    public ApplicationContext getApplicationContext() {
        return applicationContext;
    }
}
