package openbackup.system.base.common.utils;

import openbackup.system.base.common.utils.files.FileZip;

import org.apache.commons.lang3.StringUtils;
import org.junit.Assert;
import org.junit.Test;
import org.springframework.util.ResourceUtils;

import java.io.File;
import java.io.FileNotFoundException;

/**
 * 文件压缩测试类
 *
 * @author w00504341
 * @since 2023-03-25
 */
public class FileZipTest {
    /**
     * 用例场景：压缩文件
     * 前置条件：文件存在
     * 检查点：压缩成功
     */
    @Test
    public void test_zip_success() throws FileNotFoundException {
        String filePath = StringUtils.join(new String[]{"classpath:files", "test1"}, File.separator);
        String tmpPath = ResourceUtils.getFile(filePath).getPath();
        File file1 = new File(tmpPath);
        File parent = file1.getParentFile();
        String zipFilePath = parent.getPath();
        String baseName = "test1.zip";
        FileZip.zip(tmpPath, zipFilePath + File.separator + baseName, baseName);
        File rsFile = new File(zipFilePath + File.separator + baseName);
        Assert.assertTrue(rsFile.exists());
        rsFile.delete();
    }
}
