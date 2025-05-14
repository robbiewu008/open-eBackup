package com.huawei.oceanprotect.system.base.initialize.network.common;

import junit.framework.TestCase;
import nl.jqno.equalsverifier.EqualsVerifier;

import org.junit.Assert;
import org.junit.Test;

/**
 * Ipv4Resource测试类
 *
 * @author s30031954
 * @since 2023-02-27
 */
public class Ipv4ResourceTest extends TestCase {
    /**
     * 用例场景：测试类的equals和hashcode方法
     * 前置条件：无
     * 检查点：是否报错
     */
    @Test
    public void testIpv4Resource() {
        EqualsVerifier.simple().forClass(Ipv4Resource.class).verify();
        EqualsVerifier.simple().forClass(Ipv4Resource.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}