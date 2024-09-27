package openbackup.data.access.framework.core.dao;

import com.baomidou.mybatisplus.autoconfigure.MybatisPlusAutoConfiguration;

import openbackup.data.access.framework.core.dao.CopiesAntiRansomwareDao;
import openbackup.data.access.framework.core.entity.CopiesAntiRansomware;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mybatis.spring.annotation.MapperScan;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.boot.autoconfigure.sql.init.SqlInitializationAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * CopiesAntiRansomwareDao测试类
 *
 * @author f00809938
 * @version OceanCyber 300 1.2.0
 * @since 2024-01-10
 **/
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {
        DataSourceAutoConfiguration.class, MybatisPlusAutoConfiguration.class, SqlInitializationAutoConfiguration.class
})
@MapperScan(basePackages = {"openbackup.data.access.framework.core.dao"})
public class CopiesAntiRansomwareDaoTest {
    @Autowired
    private CopiesAntiRansomwareDao copiesAntiRansomwareDao;

    @Before
    public void initCopiesAntiRansomware() {
        CopiesAntiRansomware copiesAntiRansomware = new CopiesAntiRansomware();
        copiesAntiRansomware.setCopyId("1");
        copiesAntiRansomware.setStatus(3);
        copiesAntiRansomware.setGenerateType("IO_DETECT");
        copiesAntiRansomwareDao.insert(copiesAntiRansomware);
        copiesAntiRansomware = new CopiesAntiRansomware();
        copiesAntiRansomware.setCopyId("2");
        copiesAntiRansomware.setStatus(3);
        copiesAntiRansomware.setGenerateType("COPY_DETECT");
        copiesAntiRansomwareDao.insert(copiesAntiRansomware);
    }

    @After
    public void clearTable() {
        copiesAntiRansomwareDao.delete(null);
    }

    /**
     * 用例场景：快照为事中快照时返回true
     * 前置条件：无
     * 检查点：返回true
     */
    @Test
    public void should_true_when_isIoDetectCopy() {
        Assert.assertTrue(copiesAntiRansomwareDao.isIoDetectCopy("1"));
    }

    /**
     * 用例场景：快照为事后快照时返回false
     * 前置条件：无
     * 检查点：返回false
     */
    @Test
    public void should_false_when_isIoDetectCopy() {
        Assert.assertFalse(copiesAntiRansomwareDao.isIoDetectCopy("2"));
    }
}
