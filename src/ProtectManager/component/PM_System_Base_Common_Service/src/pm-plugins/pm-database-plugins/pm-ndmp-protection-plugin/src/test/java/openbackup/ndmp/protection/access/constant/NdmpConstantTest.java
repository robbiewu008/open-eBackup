/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.ndmp.protection.access.constant;

import nl.jqno.equalsverifier.EqualsVerifier;
import nl.jqno.equalsverifier.Warning;
import openbackup.ndmp.protection.access.constant.NdmpConstant;

import org.junit.Test;

/**
 * 常数类测试
 *
 * @author t30021437
 * @since 2023-05-09
 */
public class NdmpConstantTest {
    /**
     * 用例场景：常数类测试
     * 前置条件：
     * 检  查  点：无
     */
    @Test
    public void testNdmpConstant() {
        EqualsVerifier.simple()
            .forClass(NdmpConstant.class)
            .suppress(Warning.INHERITED_DIRECTLY_FROM_OBJECT)
            .suppress(Warning.ALL_NONFINAL_FIELDS_SHOULD_BE_USED)
            .verify();
        EqualsVerifier.simple()
            .forClass(NdmpConstant.class)
            .suppress(Warning.INHERITED_DIRECTLY_FROM_OBJECT)
            .suppress(Warning.ALL_NONFINAL_FIELDS_SHOULD_BE_USED)
            .usingGetClass()
            .verify();
    }
}