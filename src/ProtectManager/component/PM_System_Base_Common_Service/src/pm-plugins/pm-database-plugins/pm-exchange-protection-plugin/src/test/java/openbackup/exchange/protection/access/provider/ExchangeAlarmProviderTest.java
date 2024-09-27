package openbackup.exchange.protection.access.provider;

import openbackup.access.framework.resource.service.ProtectedResourceServiceImpl;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * Exchange Alarm Provider
 *
 * @author c30058517
 * @since 2024-04-02
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {ExchangeAlarmProvider.class, ProtectedResourceServiceImpl.class})
public class ExchangeAlarmProviderTest {

    private final ExchangeAlarmProvider provider = new ExchangeAlarmProvider();

    /**
     * 用例场景：测试资源能否执行
     * 前置条件：无
     * 检查点：检查通过
     */
    @Test
    public void testApplicable() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.EXCHANGE_MAILBOX.getType());
        Assert.assertTrue(provider.applicable(protectedResource));
    }

    /**
     * 用例场景：测试返回信息
     * 前置条件：无
     * 检查点：返回信息格式正确
     */
    @Test
    public void testGetAlarmResourceName() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.EXCHANGE_MAILBOX.getType());
        protectedResource.setParentName("parentName");
        protectedResource.setName("name");
        Assert.assertEquals("parentName+name", provider.getAlarmResourceName(protectedResource));
    }
}