package com.huawei.oceanprotect.system.base.initialize.network.beans;

import junit.framework.TestCase;
import nl.jqno.equalsverifier.EqualsVerifier;

import org.junit.Assert;

/**
 * InitNetworkResult测试类
 *
 * @author s30031954
 * @since 2023-02-27
 */
public class InitNetworkResultTest extends TestCase {
    /**
     * 用例场景：测试类的equals和hashcode方法
     * 前置条件：无
     * 检查点：是否报错
     */
    public void testNetworkResultTest() {
        EqualsVerifier.simple().forClass(InitNetworkResult.class).verify();
        EqualsVerifier.simple().forClass(InitNetworkResult.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}