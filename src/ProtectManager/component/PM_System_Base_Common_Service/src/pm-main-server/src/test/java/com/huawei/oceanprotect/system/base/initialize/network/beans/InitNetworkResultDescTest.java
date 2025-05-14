package com.huawei.oceanprotect.system.base.initialize.network.beans;

import junit.framework.TestCase;
import nl.jqno.equalsverifier.EqualsVerifier;

import org.junit.Assert;

/**
 * InitNetworkResultDesc测试类
 *
 * @author s30031954
 * @since 2023-02-27
 */
public class InitNetworkResultDescTest extends TestCase {
    /**
     * 用例场景：测试类的equals和hashcode方法
     * 前置条件：无
     * 检查点：是否报错
     */
    public void testInitNetworkResultDesc() {
        EqualsVerifier.simple().forClass(InitNetworkResultDesc.class).verify();
        EqualsVerifier.simple().forClass(InitNetworkResultDesc.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}