package com.huawei.emeistor.console.util;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.context.ApplicationContext;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-09-14
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {SystemUtil.class,ApplicationContext.class,System.class})
public class SystemUtilTest {
    @Test
    public void test_stop_application() {
        ApplicationContext applicationContextAssert = PowerMockito.mock(ApplicationContext.class);
        PowerMockito.mockStatic(System.class);
        SystemUtil.stopApplication(applicationContextAssert);
    }
}
