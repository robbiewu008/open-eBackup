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
package openbackup.system.base.util;

import static org.assertj.core.api.Assertions.assertThatIllegalArgumentException;
import static org.assertj.core.api.BDDAssertions.then;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;

import openbackup.system.base.util.SpringBeanUtils;

import org.junit.Test;
import org.mockito.Mockito;
import org.springframework.context.ApplicationContext;

import java.util.HashMap;
import java.util.Map;

/**
 * Spring Bean 工具类的测试用例
 *
 **/
public class SpringBeanUtilsTest {
    private static final ApplicationContext applicationContext = Mockito.mock(ApplicationContext.class);

    private static final SpringBeanUtils springBeanUtils = new SpringBeanUtils();

    static {
        springBeanUtils.setApplicationContext(applicationContext);
    }


    /**
     * 用例名称：验证正确的获取上下文对象<br/>
     * 前置条件：无<br/>
     * check点：1.上下文对象非空  2.上下文对象为指定对象<br/>
     */
    @Test
    public void should_return_context_when_getApplicationContext() {
        then(SpringBeanUtils.getApplicationContext()).isNotNull().isEqualTo(applicationContext);
    }

    /**
     * 用例名称：验证根据名称获取bean成功<br/>
     * 前置条件：无<br/>
     * check点：1.获取到bean为预期的对象<br/>
     */
    @Test
    public void should_return_bean_when_getBean_by_name_given_bean_exist() {
        // Given
        String beanName = "bean1";
        Object object = new Object();
        given(applicationContext.getBean(eq(beanName))).willReturn(object);
        // When
        final Object bean = SpringBeanUtils.getBean(beanName);
        // Then
        then(bean).isNotNull().isEqualTo(object);
    }

    /**
     * 用例名称：验证根据class获取bean成功<br/>
     * 前置条件：无<br/>
     * check点：1.获取到bean为预期的对象<br/>
     */
    @Test
    public void should_return_bean_when_getBean_by_class_given_class_exist() {
        // Given
        Class clazz = Class.class;
        Object object = new Object();
        given(applicationContext.getBean(eq(clazz))).willReturn(object);
        // When
        final Object bean = SpringBeanUtils.getBean(clazz);
        // Then
        then(bean).isNotNull().isEqualTo(object);
    }

    /**
     * 用例名称：验证根据class和name获取bean成功<br/>
     * 前置条件：无<br/>
     * check点：1.获取到bean为预期的对象<br/>
     */
    @Test
    public void should_return_bean_when_getBean_by_class_and_name_given_class_exist() {
        // Given
        String beanName = "bean1";
        Class clazz = Class.class;
        Object object = new Object();
        given(applicationContext.getBean(eq(beanName), eq(clazz))).willReturn(object);
        // When
        final Object bean = SpringBeanUtils.getBean(beanName, clazz);
        // Then
        then(bean).isNotNull().isEqualTo(object);
    }

    /**
     * 用例名称：验证根据class获取实现或集成自该类的<br/>
     * 前置条件：无<br/>
     * check点：1.获取到bean为预期的对象<br/>
     */
    @Test
    public void should_return_bean_when_getBeans_by_class_and_name_given_classes_exist() {
        // Given
        Class clazz = Class.class;
        Object object1 = new Object();
        Object object2 = new Object();
        Object object3 = new Object();
        Map<String, Object> resultMap = new HashMap<>();
        resultMap.put("bean1", object1);
        resultMap.put("bean2", object2);
        resultMap.put("bean3", object3);
        given(applicationContext.getBeansOfType(eq(clazz))).willReturn(resultMap);
        // When
        final Map beansByClass = SpringBeanUtils.getBeansByClass(clazz);
        // Then
        then(beansByClass).isNotEmpty().isSameAs(resultMap);
    }

    /**
     * 用例名称：验证根据class为空时，抛出异常<br/>
     * 前置条件：无<br/>
     * check点：1.异常为IllegalArgumentException 2.异常信息为期望信息<br/>
     */
    @Test
    public void should_throw_IllegalArgumentException_when_getBeans_by_class_and_name_class_is_null() {
        // When and Then
        assertThatIllegalArgumentException().isThrownBy(() -> SpringBeanUtils.getBeansByClass(null))
            .withMessage("class is empty");
    }
}
