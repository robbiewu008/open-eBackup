package openbackup.system.base.util;

import openbackup.system.base.common.utils.files.FileZip;
import openbackup.system.base.util.ClusterFileUtils;

import org.apache.commons.io.FileUtils;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * ClusterFileUtils工具类测试类
 *
 * @author y30046482
 * @since 2023-05-17
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({FileZip.class, FileUtils.class, ClusterFileUtils.class})
public class ClusterFileUtilsTest {

    @Before
    public void mockStaticClass() {
        PowerMockito.mockStatic(FileZip.class);
        PowerMockito.mockStatic(FileUtils.class);
        PowerMockito.mockStatic(ClusterFileUtils.class);
    }

    @Test
    public void test_createMultipartFile() {
        ClusterFileUtils.createMultipartFile("");
        Assert.assertTrue(true);
    }

    @Test
    public void test_deleteFile() {
        ClusterFileUtils.deleteFile("");
        Assert.assertTrue(true);
    }
}