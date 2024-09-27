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

import openbackup.system.base.service.ApplicationContextService;
import openbackup.system.base.util.Applicable;

import lombok.Builder;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.context.ApplicationContext;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Collections;

/**
 * Application Context Service Test
 *
 * @author twx1009756
 * @since 2021-03-17
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {ApplicationContextService.class})
public class ApplicationContextServiceTest {
    @Autowired
    ApplicationContextService applicationContextService;

    /**
     * 测试autowired注入bean方法
     */
    @Test
    public void applicationContextService_autowired_to_bean_success() {
        String name = "Lisa";
        String gender = "Woman";
        Person person =
                applicationContextService.autowired(
                        Person.builder().gender(gender).name(name).build(),
                        Collections.singletonList(Mockito.mock(Applicable.class)));
        Assert.assertNotNull(person);
    }

    @Builder
    private static class Person {
        private String gender;

        private String name;

        @Override
        public String toString() {
            return "Person{" + "name='" + name + '\'' + ", " + "gender='" + gender + '\'' + '}';
        }
    }

    /**
     * 测试得到应用上下文方法
     */
    @Test
    public void get_applicationContext_test() {
        ApplicationContext applicationContext = applicationContextService.getApplicationContext();
        Assert.assertNotNull(applicationContext);
    }
}
