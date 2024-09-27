package openbackup.data.access.framework.livemount.provider;

import openbackup.data.access.framework.livemount.provider.VoidLiveMountInterceptorProvider;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

/**
 * Void Live Mount Interceptor Provider Test
 *
 * @author l00272247
 * @since 2021-12-30
 */
public class VoidLiveMountInterceptorProviderTest {
    @Test
    public void test_applicable() {
        VoidLiveMountInterceptorProvider provider = new VoidLiveMountInterceptorProvider();
        Assertions.assertTrue(provider.applicable(null));
        Assertions.assertFalse(provider.applicable(""));
    }
}
