package openbackup.system.base.util;

import openbackup.system.base.util.StreamUtil;

import org.junit.Assert;
import org.junit.Test;

import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Stream Util Test
 *
 * @author l00650874
 * @since 2022-07-01
 */
public class StreamUtilTest {
    /**
     * 用例名称：验证match方法匹配逻辑是否正确。<br/>
     * 前置条件：Stream对象初始化完成。<br/>
     * check点：<br/>
     * 1、正常将类型匹配的元数过滤出来；<br/>
     */
    @Test
    public void test_match() {
        String result = Stream.of(0, "1", "2", "3").flatMap(StreamUtil.match(String.class)).collect(Collectors.joining());
        Assert.assertEquals("123", result);
    }
}
