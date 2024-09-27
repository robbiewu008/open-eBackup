package openbackup.system.base.util;

import openbackup.system.base.util.OptionalUtil;

import org.junit.Assert;
import org.junit.Test;

import java.util.Optional;

/**
 * Optional Util Test
 *
 * @author l00650874
 * @since 2022-07-01
 */
public class OptionalUtilTest {
    /**
     * 用例名称：验证match方法匹配逻辑是否正确。<br/>
     * 前置条件：Optional对象初始化完成。<br/>
     * check点：<br/>
     * 1、正常将类型匹配的元数过滤出来；<br/>
     */
    @Test
    public void test_match() {
        Assert.assertFalse(Optional.of(0).flatMap(OptionalUtil.match(String.class)).isPresent());
        Assert.assertTrue(Optional.of("1").flatMap(OptionalUtil.match(CharSequence.class)).isPresent());
    }
}
