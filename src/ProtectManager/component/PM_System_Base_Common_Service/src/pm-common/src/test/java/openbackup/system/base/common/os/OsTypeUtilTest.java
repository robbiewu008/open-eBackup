package openbackup.system.base.common.os;

import openbackup.system.base.common.os.enums.OsType;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * 测试操作系统类型工具
 *
 * @author w00493811
 * @since 2021-08-09
 */
@RunWith(SpringRunner.class)
public class OsTypeUtilTest {
    @Test
    public void test_is_windows() {
        OsTypeHelper.modifyOsTypeUtilOsName(OsType.WINDOWS);
        Assert.assertEquals(OsType.WINDOWS, OsTypeUtil.getOsType());
    }

    @Test
    public void test_is_linux() {
        OsTypeHelper.modifyOsTypeUtilOsName(OsType.LINUX);
        Assert.assertEquals(OsType.LINUX, OsTypeUtil.getOsType());
    }

    @Test
    public void test_is_other() {
        OsTypeHelper.modifyOsTypeUtilOsName(OsType.OTHERS);
        Assert.assertEquals(OsType.OTHERS, OsTypeUtil.getOsType());
    }
}
