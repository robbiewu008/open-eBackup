package openbackup.data.access.framework.core.common.util;

import openbackup.system.base.common.exception.DataMoverCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;

import com.fasterxml.jackson.core.type.TypeReference;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Objects;

public class JsonUtilTest {

    private final String jsonStr = "{\"name\":\"test\"}";

    /**
     * 验证对象转json的正常和异常场景
     */
    @Test
    public void test_json() {
        Bean bean = new Bean();
        bean.setName("test");
        String json = JsonUtil.json(bean);
        Assert.assertEquals(jsonStr, json);
    }

    /**
     * 验证json字符串转对象
     */
    @Test
    public void test_read() throws IOException {
        Bean read = JsonUtil.read(jsonStr, Bean.class);
        Bean bean = new Bean();
        bean.setName("test");
        Assert.assertEquals(bean, read);
    }

    /**
     * 验证输入流转json node的正常和异常场景
     */
    @Test(expected = DataMoverCheckedException.class)
    public void test_read_by_inputStream() throws IOException {
        byte[] bytes = jsonStr.getBytes(StandardCharsets.UTF_8);
        JsonUtil.read(new ByteArrayInputStream(bytes));
        ByteArrayInputStream inputStream = PowerMockito.spy(new ByteArrayInputStream(bytes));
        PowerMockito.doThrow(new IOException("test")).when(inputStream).close();
        JsonUtil.read(inputStream);
    }

    /**
     * 验证类型转换
     */
    @Test
    public void test_cast() {
        String cast = JsonUtil.cast(new Integer(1), String.class);
        Assert.assertEquals("1", cast);
    }

    /**
     * 验证json 数据转换成对象
     */
    @Test
    public void test_read_with_json_data_and_typeReference() {
        Bean read1 = JsonUtil.read(jsonStr, new TypeReference<Bean>() {
        });
        Bean bean = new Bean();
        bean.setName("test");
        Bean read2 = JsonUtil.read(bean, new TypeReference<Bean>() {
        });
        Assert.assertEquals(bean, read1);
        Assert.assertEquals(bean, read2);
    }

    private static class Bean {
        String name;

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;

            Bean bean = (Bean) o;

            return Objects.equals(name, bean.name);
        }

        @Override
        public int hashCode() {
            return name != null ? name.hashCode() : 0;
        }
    }
}
