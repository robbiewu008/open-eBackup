package openbackup.system.base.common.aspect;

import openbackup.system.base.common.aspect.DateConverter;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;
import javax.annotation.Resource;

/**
 * Date Converter Test
 *
 * @author c30044692
 * @since 2023-04-20
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = DateConverter.class)
public class DateConverterTest {

    @Resource
    private DateConverter dateConverter;

    /**
     * 测试场景：当data为null时，测试cast是否正常 <br/>
     * 前置条件：data为null <br/>
     * 检查点：cast返回null
     */
    @Test
    public void cast_should_return_null_when_data_is_null () {
        Assert.assertNull(dateConverter.cast(null));
    }

    /**
     * 测试场景：当data不为null时，测试cast是否正常 <br/>
     * 前置条件：data不为null <br/>
     * 检查点：cast正常返回
     */
    @Test
    public void cast_should_return_format_string_when_data_is_not_null () {
        Assert.assertEquals("2012-01-29 22:26:23", dateConverter.cast("2012-01-29 22:26:23"));
        Assert.assertNull(dateConverter.cast("2012-01-29"));
        Assert.assertEquals("2023-04-20 14:21:55", dateConverter.cast(1681971715000L));
        Assert.assertNull(dateConverter.cast(new Object()));
    }
}
