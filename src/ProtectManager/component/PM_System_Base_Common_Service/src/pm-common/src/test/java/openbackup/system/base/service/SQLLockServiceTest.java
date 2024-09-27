
package openbackup.system.base.service;

import openbackup.system.base.pack.lock.SQLLockService;
import openbackup.system.base.pack.lock.mapper.LockMapper;
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

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-06-12
 */
@RunWith(PowerMockRunner.class)
@SpringBootTest(classes = {DataSourceAutoConfiguration.class, MybatisPlusAutoConfiguration.class, SQLLockService.class,
        TransactionAutoConfiguration.class})
@MapperScan("com.huawei.oceanprotect.system.base.pack.lock.mapper")
@PowerMockRunnerDelegate(SpringRunner.class)
public class SQLLockServiceTest {
    @Autowired
    private SQLLockService sqlLockService;

    @MockBean
    private LockMapper lockMapper;

    @Test
    public void test_create_lock_success() {
        PowerMockito.when(lockMapper.insert(Mockito.any())).thenReturn(1);
        boolean demo = sqlLockService.createLock("test_demo_1");
        Assert.assertTrue(demo);
    }

    @Test
    public void test_unlock_all() {
        sqlLockService.afterSingletonsInstantiated();
    }
}
