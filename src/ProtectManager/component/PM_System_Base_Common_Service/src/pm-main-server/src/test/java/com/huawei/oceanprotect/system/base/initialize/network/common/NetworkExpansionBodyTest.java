package com.huawei.oceanprotect.system.base.initialize.network.common;

import junit.framework.TestCase;
import nl.jqno.equalsverifier.EqualsVerifier;

import org.junit.Assert;
import org.junit.Test;

/**
 * NetworkExpansionBody测试类
 *
 * @author s30031954
 * @since 2023-02-27
 */
public class NetworkExpansionBodyTest extends TestCase {
    /**
     * 用例场景：测试类的equals和hashcode方法
     * 前置条件：无
     * 检查点：是否报错
     */
    @Test
    public void testNetworkExpansionBody() {
        EqualsVerifier.simple().forClass(NetworkExpansionBody.class).verify();
        EqualsVerifier.simple().forClass(NetworkExpansionBody.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}