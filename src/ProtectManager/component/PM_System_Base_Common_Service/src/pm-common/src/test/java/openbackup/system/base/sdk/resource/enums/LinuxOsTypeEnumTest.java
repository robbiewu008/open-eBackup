package openbackup.system.base.sdk.resource.enums;

import openbackup.system.base.sdk.resource.enums.LinuxOsTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 * @author s30036254
 * @since 2023-01-17
 */
public class LinuxOsTypeEnumTest {
    /**
     * 测试用例：测试枚举类的功能
     * 前置条件：无
     * CHECK点：功能是否正常调用
     */
    @Test
    public void testEnum() {
        Assert.assertEquals(LinuxOsTypeEnum.REDHAT.getName(),"RedHat");
        Assert.assertEquals(LinuxOsTypeEnum.SUSE.getName(),"SUSE");
        Assert.assertEquals(LinuxOsTypeEnum.ROCKY.getName(),"ROCKY");
        Assert.assertEquals(LinuxOsTypeEnum.OEL.getName(),"OEL");
        Assert.assertEquals(LinuxOsTypeEnum.ISOFT.getName(),"ISOFT");
        Assert.assertEquals(LinuxOsTypeEnum.CENTOS.getName(),"CentOS");
        Assert.assertEquals(LinuxOsTypeEnum.KYLIN.getName(),"Kylin");
        Assert.assertEquals(LinuxOsTypeEnum.NEO_KYLIN.getName(),"NeoKylin");
        Assert.assertEquals(LinuxOsTypeEnum.UNION_TECH_OS.getName(),"UnionTech OS");
        Assert.assertEquals(LinuxOsTypeEnum.OPEN_EULER.getName(),"openEuler");
        Assert.assertEquals(LinuxOsTypeEnum.DEBIAN.getName(),"Debian");
        Assert.assertEquals(LinuxOsTypeEnum.SOLARIS.getName(),"SOLARIS");
        Assert.assertEquals(LinuxOsTypeEnum.HPUXIA.getName(),"HPUXIA");
        Assert.assertEquals(LinuxOsTypeEnum.UBUNTU.getName(),"Ubuntu");
    }

    /**
     * 测试用例：测试是否是linux
     * 前置条件：无
     * CHECK点：功能是否正常调用
     */
    @Test
    public void test_isLinuxType() {
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.REDHAT.getName()));
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.SUSE.getName()));
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.ROCKY.getName()));
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.OEL.getName()));
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.ISOFT.getName()));
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.CENTOS.getName()));
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.KYLIN.getName()));
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.NEO_KYLIN.getName()));
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.UNION_TECH_OS.getName()));
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.OPEN_EULER.getName()));
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.DEBIAN.getName()));
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.SOLARIS.getName()));
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.HPUXIA.getName()));
        Assert.assertTrue(LinuxOsTypeEnum.isLinuxType(LinuxOsTypeEnum.UBUNTU.getName()));
    }
}
