package com.huawei.oceanprotect.system.base.initialize.network.beans;

import junit.framework.TestCase;
import nl.jqno.equalsverifier.EqualsVerifier;
import nl.jqno.equalsverifier.Warning;

import org.junit.Assert;

/**
 * InitRouteInfoTest测试类
 *
 * @author s30031954
 * @since 2023-02-27
 */
public class InitRouteInfoTest extends TestCase {
    /**
     * 用例场景：测试类的equals和hashcode方法
     * 前置条件：无
     * 检查点：是否报错
     */
    public void testInitRouteInfo() {
        EqualsVerifier.simple()
            .forClass(InitRouteInfo.class)
            .suppress(Warning.INHERITED_DIRECTLY_FROM_OBJECT)
            .suppress(Warning.ALL_NONFINAL_FIELDS_SHOULD_BE_USED)
            .verify();
        EqualsVerifier.simple()
            .forClass(InitRouteInfo.class)
            .suppress(Warning.INHERITED_DIRECTLY_FROM_OBJECT)
            .suppress(Warning.ALL_NONFINAL_FIELDS_SHOULD_BE_USED)
            .usingGetClass()
            .verify();
        Assert.assertTrue(true);
    }
}