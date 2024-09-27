package openbackup.data.access.framework.core.manager;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.system.base.common.exception.DataMoverCheckedException;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.context.ApplicationContext;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * Provider Manager Test
 *
 * @author l00272247
 * @since 2022-01-17
 */
public class ProviderManagerTest {
    /**
     * 用例名称：验证获取DataProtectionProvider的正常异常场景。<br/>
     * 前置条件：ProviderManager初始化成功。<br/>
     * check点：<br/>
     * 1、对应provider缺失的情况下，使用默认provider；<br/>
     * 2、对应provider存在的情况下，使用已有provider。<br/>
     */
    @Test
    public void test_find_provider_or_default() {
        ApplicationContext applicationContext = PowerMockito.mock(ApplicationContext.class);
        ProviderManager providerManager = new ProviderManager(applicationContext);
        FakeDataProtectionProvider provider1 = new FakeDataProtectionProvider();
        FakeDataProtectionProvider provider2 = new FakeDataProtectionProvider();
        PowerMockito.when(applicationContext.getBeansOfType(any()))
                .thenReturn(Collections.singletonMap("x", provider2));
        Assert.assertSame(
                provider1, providerManager.findProviderOrDefault(FakeDataProtectionProvider.class, "", provider1));
        Assert.assertSame(
                provider2, providerManager.findProviderOrDefault(FakeDataProtectionProvider.class, "x", provider1));
    }

    /**
     * 用例名称：验证获取所有DataProtectionProvider的正常异常场景。<br/>
     * 前置条件：ProviderManager初始化成功。<br/>
     * check点：<br/>
     * 1、所有provider满足条件的情况下，返回所有provider；<br/>
     * 2、存在不满足条件的provider的情况下，剔除不满足条件的provider。<br/>
     */
    @Test
    public void test_find_providers_with_identification() {
        ApplicationContext applicationContext = PowerMockito.mock(ApplicationContext.class);
        ProviderManager providerManager = new ProviderManager(applicationContext);
        FakeDataProtectionProvider provider1 = new FakeDataProtectionProvider();
        FakeDataProtectionProvider provider2 = new FakeDataProtectionProvider();
        FakeDataProtectionProvider provider3 = new OtherFakeDataProtectionProvider();
        Map<String, FakeDataProtectionProvider> providerMap = new HashMap<>();
        providerMap.put("provider1", provider1);
        providerMap.put("provider2", provider2);
        providerMap.put("provider3", provider3);
        PowerMockito.when(applicationContext.getBeansOfType(FakeDataProtectionProvider.class)).thenReturn(providerMap);
        Collection<FakeDataProtectionProvider> providers = providerManager.findProviders(
                FakeDataProtectionProvider.class, "x");
        Assert.assertEquals(2, providers.size());
        Assert.assertTrue(providers.contains(provider1));
        Assert.assertTrue(providers.contains(provider2));
        Assert.assertFalse(providers.contains(provider3));
    }

    /**
     * 用例名称：验证获取DataProtectionProvider的异常场景。<br/>
     * 前置条件：ProviderManager初始化成功。<br/>
     * check点：<br/>
     * 1、无满足条件的provider时，抛出异常。<br/>
     */
    @Test(expected = DataMoverCheckedException.class)
    public void test_find_provider_with_identification_failed() {
        ApplicationContext applicationContext = PowerMockito.mock(ApplicationContext.class);
        ProviderManager providerManager = new ProviderManager(applicationContext);
        FakeDataProtectionProvider provider1 = new OtherFakeDataProtectionProvider();
        Map<String, FakeDataProtectionProvider> providerMap = new HashMap<>();
        providerMap.put("provider1", provider1);
        PowerMockito.when(applicationContext.getBeansOfType(FakeDataProtectionProvider.class)).thenReturn(providerMap);
        FakeDataProtectionProvider providerTest = providerManager.findProvider(
                FakeDataProtectionProvider.class, "x");
    }

    private static class FakeDataProtectionProvider implements DataProtectionProvider<String> {
        @Override
        public boolean applicable(String object) {
            return "x".equals(object);
        }
    }

    private static class OtherFakeDataProtectionProvider extends FakeDataProtectionProvider {
        @Override
        public boolean applicable(String object) {
            return "Y".equals(object);
        }
    }
}
