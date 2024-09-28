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

import openbackup.system.base.service.ServiceManager;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.context.ApplicationContext;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * Service Manager Test
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {
    ServiceManager.class, ApplicationContext.class
})
public class ServiceManagerTest {
    @Autowired
    ServiceManager serviceManager;

    @Autowired
    ApplicationContext applicationContext;

    /**
     * 测试getService方法
     */
   @Test
   public void test_getService(){
       serviceManager.getService(ApplicationContext.class, "serviceName");
       serviceManager.afterPropertiesSet();
       serviceManager.setApplicationContext(applicationContext);
   }
}
