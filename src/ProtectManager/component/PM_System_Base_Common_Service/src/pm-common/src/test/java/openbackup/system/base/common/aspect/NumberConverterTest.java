package openbackup.system.base.common.aspect;

import openbackup.system.base.common.aspect.NumberConverter;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;
import javax.annotation.Resource;

/**
 * Number Converter Test
 *
 * @author c30044692
 * @since 2023/4/20
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = NumberConverter.class)
public class NumberConverterTest {

    @Resource
    private NumberConverter numberConverter;


    /**
     * 测试场景：能正确处理入参为空，包含数字和字符串等情况 <br/>
     * 前置条件：入参为null <br/>
     * 检查点：正确处理，如果不能转换为字符串返回null
     */
    @Test
    public void test_cast () {
        Assert.assertNull(numberConverter.cast(null));
        Assert.assertEquals(12L, numberConverter.cast("12"));
        Assert.assertEquals(12D, numberConverter.cast("12.0"));
        Assert.assertNull(numberConverter.cast("12.0CCCC"));
        Assert.assertNull(numberConverter.cast(new Object()));
    }
}