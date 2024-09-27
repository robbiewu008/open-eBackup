package openbackup.system.base.util;

import openbackup.system.base.util.ThreadCache;

import org.junit.Assert;
import org.junit.Test;

/**
 * Thread Cache Test
 * 
 * @author l00272247
 * @since 2021-11-23
 */
public class ThreadCacheTest {
    @Test
    public void test() {
        ThreadCache<Object> cache = new ThreadCache<>();
        Object[] data = new Object[] {new Object(), new Object()};
        cache.run(() -> {
            Assert.assertSame(data[0], cache.get());
            cache.run(() -> Assert.assertSame(data[1], cache.get()), data[1]);
            Assert.assertSame(data[0], cache.get());
        }, data[0]);
        Assert.assertNull(cache.get());
    }
}
