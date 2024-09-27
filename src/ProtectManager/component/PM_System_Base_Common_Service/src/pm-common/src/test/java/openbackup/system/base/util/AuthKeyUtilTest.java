package openbackup.system.base.util;

import openbackup.system.base.util.AuthKeyUtil;

import org.junit.Assert;
import org.junit.Test;

/**
 * AuthKeyUtilTest
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-26
 */
public class AuthKeyUtilTest {
    /**
     * 用例场景：生成符合要求的随机私钥，12位
     * 前置条件：服务正常
     * 检查点：返回一个随机私钥，12位
     */
    @Test
    public void get_private_key_success() {
        String privateKey = AuthKeyUtil.genPrivateKey();
        String privateKey2 = AuthKeyUtil.genPrivateKey();
        Assert.assertNotNull(privateKey);
        Assert.assertEquals(12, privateKey.length());
        Assert.assertEquals(12, privateKey2.length());
        Assert.assertNotEquals(privateKey, privateKey2);
    }
}
