package openbackup.system.base.util;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.util.SensitiveValidateUtil;

import org.junit.Assert;
import org.junit.Test;

import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * 功能描述
 *
 * @author t30028453
 * @since 2022-10-12
 */
public class SensitiveValidateUtilTest {
    /**
     * 用例名称：敏感信息校验工具，抛出异常
     * 前置条件：相应方法已mock
     * check点：有敏感信息时会抛出异常
     */
    @Test
    public void should_throw_error_when_fields_sensitive() {
        List<String> toValidateStrs = Stream.of(new String[]{"pwd", "password"}).collect(Collectors.toList());

        LegoCheckedException exception =
                Assert.assertThrows(LegoCheckedException.class, () -> SensitiveValidateUtil.doValidate(toValidateStrs));
        Assert.assertEquals(exception.getMessage(), "The sensitive field name is found. Please confirm.");
    }
}