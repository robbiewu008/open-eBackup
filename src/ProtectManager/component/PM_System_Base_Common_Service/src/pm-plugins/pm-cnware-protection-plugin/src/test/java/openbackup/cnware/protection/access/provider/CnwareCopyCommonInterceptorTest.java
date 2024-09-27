package openbackup.cnware.protection.access.provider;

import openbackup.cnware.protection.access.provider.CnwareCopyCommonInterceptor;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;

/**
 * CnwareCopyCommonInterceptor
 *
 * @author y30037959
 * @since 2024-08-24
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(CnwareCopyCommonInterceptor.class)
@AutoConfigureMockMvc
public class CnwareCopyCommonInterceptorTest {
    @Test
    public void test_applicable() {
        CnwareCopyCommonInterceptor provider = new CnwareCopyCommonInterceptor();
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.CNWARE_VM.getType()));
    }
}