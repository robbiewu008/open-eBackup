package com.huawei.oceanprotect.system.base.initialize.network.beans;

import junit.framework.TestCase;
import nl.jqno.equalsverifier.EqualsVerifier;

import org.junit.Assert;

/**
 * NetworkExpansionStatusTest测试类
 *
 * @author s30031954
 * @since 2023-02-27
 */
public class NetworkExpansionStatusTest extends TestCase {
    /**
     * 用例场景：测试类的equals和hashcode方法
     * 前置条件：无
     * 检查点：是否报错
     */
    public void testNetworkExpansionStatus() {
        EqualsVerifier.simple().forClass(NetworkExpansionStatus.class).verify();
        EqualsVerifier.simple().forClass(NetworkExpansionStatus.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}