/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * @author twx1009756
 * @since 2021-03-17
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
