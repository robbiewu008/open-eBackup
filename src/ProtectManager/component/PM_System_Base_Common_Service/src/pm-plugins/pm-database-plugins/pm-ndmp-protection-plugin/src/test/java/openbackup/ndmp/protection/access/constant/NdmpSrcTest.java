package openbackup.ndmp.protection.access.constant;

import nl.jqno.equalsverifier.EqualsVerifier;
import nl.jqno.equalsverifier.Warning;
import openbackup.ndmp.protection.access.constant.NdmpSrc;

import org.junit.Test;

/**
 * bean 测试
 *
 * @author t30021437
 * @since 2023-05-18
 */
public class NdmpSrcTest {
    /**
     * 用例场景：常数类测试
     * 前置条件：
     * 检  查  点：无
     */
    @Test
    public void testNdmpConstant() {
        EqualsVerifier.simple()
            .forClass(NdmpSrc.class)
            .suppress(Warning.INHERITED_DIRECTLY_FROM_OBJECT)
            .suppress(Warning.ALL_NONFINAL_FIELDS_SHOULD_BE_USED)
            .verify();
        EqualsVerifier.simple()
            .forClass(NdmpSrc.class)
            .suppress(Warning.INHERITED_DIRECTLY_FROM_OBJECT)
            .suppress(Warning.ALL_NONFINAL_FIELDS_SHOULD_BE_USED)
            .usingGetClass()
            .verify();
    }
}