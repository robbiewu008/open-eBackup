package openbackup.data.access.framework.livemount.convert;

import openbackup.data.access.framework.livemount.converter.LiveMountDataConverter;
import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Collection;

/**
 * test LiveMountData Converter
 *
 * @author fwx1022842
 * @since 2021-3-8
 */
@RunWith(SpringRunner.class)
@SpringBootTest
@ContextConfiguration(classes = {LiveMountDataConverter.class})
public class LiveMountDataConverterTest {
    @MockBean
    private LiveMountEntityDao liveMountEntityDao;

    @Autowired
    private LiveMountDataConverter liveMountDataConverter;

    /**
     * get name
     */
    @Test
    public void getName() {
        String name = liveMountDataConverter.getName();
        assert "live_mount".equals(name);
    }

    /**
     * convert
     */
    @Test
    public void convert() {
        Collection<String> data = new ArrayList<>();
        data.add("3");
        data.add("4");
        Collection<?> convert = liveMountDataConverter.convert(data);
        Assert.assertEquals(convert.size(), 0);
    }
}
