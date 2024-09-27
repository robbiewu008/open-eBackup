/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.context.ApplicationContext;

import java.util.Collections;

import static org.mockito.ArgumentMatchers.any;

/**
 * Provider Registry Test
 *
 * @author twx1009756
 * @since 2021-03-17
 */
@RunWith(MockitoJUnitRunner.class)
@SpringBootTest(classes = {
    ProviderRegistry.class, ApplicationContext.class, EnumUtil.class, ProviderRegistryTest.TestProvider.class
})
public class ProviderRegistryTest {
    @Autowired
    @InjectMocks
    ProviderRegistry providerRegistry;

    @Autowired
    @Mock
    ApplicationContext applicationContext;

    /**
     * 测试FindProvider方法
     */
    @Test
    public void test_find_provider() {
        try {
            providerRegistry.findProvider(TestProvider.class, "enumUtil");
        } catch (LegoCheckedException ex) {
            ex.printStackTrace();
        }
    }

    /**
     * 用例名称：验证获取Applicable的正常异常场景。<br/>
     * 前置条件：ProviderRegistry初始化成功。<br/>
     * check点：<br/>
     * 1、对应Applicable缺失的情况下，使用默认Applicable；<br/>
     * 2、对应Applicable存在的情况下，使用已有Applicable。<br/>
     */
    @Test
    public void test_find_provider_or_default() {
        TestProvider provider1 = new TestProvider();
        TestProvider provider2 = new TestProvider();
        PowerMockito.when(applicationContext.getBeansOfType(any()))
                .thenReturn(Collections.singletonMap("enumUtil", provider2));
        Assert.assertSame(
                provider1, providerRegistry.findProviderOrDefault(TestProvider.class, "", provider1));
        Assert.assertSame(
                provider2, providerRegistry.findProviderOrDefault(TestProvider.class, "enumUtil", provider1));
    }

    static class TestProvider implements Applicable<String> {
        @Override
        public boolean applicable(String object) {
            return "enumUtil".equals(object);
        }
    }
}
