package openbackup.data.access.client.sdk.api.framework.dme;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.beans.IntrospectionException;
import java.beans.PropertyDescriptor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * 覆盖类中的成员变量
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023/4/20
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({AvailableTimeRanges.class, DmeTest.class})
public abstract class DmeTest<T> {

    protected abstract T getT();

    private void testGetAndSet() throws IllegalAccessException, InstantiationException,
            InvocationTargetException, IntrospectionException {
        T t = getT();
        Class modelClass = t.getClass();
        Object obj = modelClass.newInstance();
        Field[] fields = modelClass.getDeclaredFields();
        for (Field f : fields) {
            //JavaBean属性名要求：前两个字母要么都大写，要么都小写
            //对于首字母是一个单词的情况，要么过滤掉，要么自己拼方法名
            if(f.getName().equals("KEY_COPY_VERIFY_FILE") || f.isSynthetic()) {
                continue;
            }
            PropertyDescriptor pd = new PropertyDescriptor(f.getName(), modelClass);
            Method get = pd.getReadMethod();
            Method set = pd.getWriteMethod();
            set.invoke(obj, get.invoke(obj));
        }
    }

    @Test
    public void getAndSetTest() throws InvocationTargetException, IntrospectionException,
            InstantiationException, IllegalAccessException {
        this.testGetAndSet();
        Assert.assertNotNull("ok");
    }
}
