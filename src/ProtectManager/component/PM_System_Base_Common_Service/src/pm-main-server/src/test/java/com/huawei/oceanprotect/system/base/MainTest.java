package com.huawei.oceanprotect.system.base;

import com.huawei.oceanprotect.Main;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.SpringApplication;

/**
 * 功能描述
 *
 * @author x30046484
 * @since 2024-01-20
 */

@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {SpringApplication.class})
public class MainTest {

    @Test
    public void test_main(){
        PowerMockito.mockStatic(SpringApplication.class);
        Main.main(new String[]{});
    }
}
