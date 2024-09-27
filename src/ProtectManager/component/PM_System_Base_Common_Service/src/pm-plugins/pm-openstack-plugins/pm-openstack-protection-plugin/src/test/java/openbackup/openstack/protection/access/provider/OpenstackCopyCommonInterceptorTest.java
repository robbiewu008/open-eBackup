package openbackup.openstack.protection.access.provider;

import openbackup.openstack.protection.access.provider.OpenstackCopyCommonInterceptor;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;

/**
 * OpenstackCopyCommonInterceptor
 *
 * @author y30037959
 * @since 2024-08-24
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(OpenstackCopyCommonInterceptor.class)
@AutoConfigureMockMvc
public class OpenstackCopyCommonInterceptorTest {

    @Test
    public void applicable() {
        OpenstackCopyCommonInterceptor provider = new OpenstackCopyCommonInterceptor();
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType()));
    }
}