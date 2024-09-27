package openbackup.system.base.config;

import openbackup.system.base.config.RedissonClientConfig;

import org.junit.Assert;
import org.junit.Test;
import org.springframework.test.util.ReflectionTestUtils;

/**
 * RedissonClientConfigTest
 */
public class RedissonClientConfigTest {

    /**
     * 用例场景：域名底座正常转为IP底座
     * 前置条件：域名底座
     * 检查点：转换成功
     */
    @Test
    public void testConvertServerAddress() {
        String serverAddress = "rediss://localhost:6369";
        RedissonClientConfig redissionClientConfig = new RedissonClientConfig();
        Object convertedServerAddress = ReflectionTestUtils.invokeMethod(redissionClientConfig, "convertServerAddress",
            serverAddress);
        Assert.assertEquals("rediss://127.0.0.1:6369", convertedServerAddress);
    }
}