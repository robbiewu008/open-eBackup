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

import openbackup.system.base.common.annotation.Adapter;

import org.springframework.beans.BeansException;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.stereotype.Service;

import java.util.HashMap;
import java.util.Map;

/**
 * 外部认证类
 *
 * @author t00482481
 * @since 2020-08-10
 */
@Service
public class ServiceManager implements InitializingBean, ApplicationContextAware {
    private final Map<String, Object> serviceImplMap = new HashMap<>();

    private ApplicationContext applicationContext;

    /**
     * 根据枚举获取对应的实现类
     *
     * @param abstractClass 抽象类
     * @param serviceName   service名称
     * @return 实现类
     */
    public <T> T getService(Class<T> abstractClass, String serviceName) {
        return (T) serviceImplMap.get(serviceName + "_" + abstractClass.getSimpleName());
    }

    @Override
    public void afterPropertiesSet() {
        Map<String, Object> beanMap = applicationContext.getBeansWithAnnotation(Adapter.class);
        // 遍历该接口的所有实现，将其放入map中
        for (Object serviceImpl : beanMap.values()) {
            Adapter adapter = serviceImpl.getClass().getAnnotation(Adapter.class);
            for (String name : adapter.names()) {
                serviceImplMap.put(name + "_" + adapter.abstractClass(), serviceImpl);
            }
        }
    }

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
        this.applicationContext = applicationContext;
    }
}
