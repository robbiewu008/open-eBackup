package com.huawei.emeistor.console.config;

import com.huawei.emeistor.console.config.lock.SQLLockService;

import com.baomidou.mybatisplus.autoconfigure.MybatisPlusAutoConfiguration;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.mybatis.spring.annotation.MapperScan;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.boot.autoconfigure.transaction.TransactionAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.transaction.support.TransactionTemplate;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-06-12
 */
@RunWith(PowerMockRunner.class)
@SpringBootTest(classes = {
    DataSourceAutoConfiguration.class, MybatisPlusAutoConfiguration.class, SQLLockService.class,
    TransactionAutoConfiguration.class
})
@MapperScan("com.huawei.emeistor.**.mapper")
@PowerMockRunnerDelegate(SpringRunner.class)
public class SQLLockServiceTest {

    @Autowired
    private SQLLockService sqlLockService;

    @MockBean
    private TransactionTemplate transactionTemplate;

    @Test
    public void test_create_lock_success() {
        PowerMockito.when(transactionTemplate.execute(Mockito.any())).thenReturn(true);
        boolean demo = sqlLockService.createLock("test_demo_1");
        Assert.assertTrue(demo);
    }

    @Test
    public void test_create_lock_fail() {
        PowerMockito.when(transactionTemplate.execute(Mockito.any())).thenReturn(false);
        boolean demo = sqlLockService.createLock("test_demo_2");
        Assert.assertFalse(demo);
    }

    @Test
    public void test_update_record() {
        PowerMockito.when(transactionTemplate.execute(Mockito.any())).thenReturn(false);
        boolean demo = sqlLockService.updateRecord("test_demo_2");
        Assert.assertFalse(demo);
    }
}
