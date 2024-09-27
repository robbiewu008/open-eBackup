package openbackup.system.base.util;

import openbackup.system.base.bean.FileCheckRule;
import openbackup.system.base.common.utils.files.FileUtil;
import openbackup.system.base.common.utils.files.FileZip;
import openbackup.system.base.util.ClusterFileUtils;

import org.apache.commons.lang3.StringUtils;
import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.springframework.util.ResourceUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

/**
 * 功能描述
 *
 * @author w00504341
 * @since 2023-02-06
 */
public class FileZipTest {
    /**
     * 用例场景：打包指定文件
     * 前置条件：无
     * 检查点：打包成功
     */
    @Test
    @Ignore
    public void test_zip_multi_spec_files_success() throws IOException {
        String zipPath = "";
        try {
            String file1Path = StringUtils.join(new String[]{"classpath:files", "test1"}, File.separator);
            String file1 = ResourceUtils.getFile(file1Path).getPath();
            String file2Path = StringUtils.join(new String[]{"classpath:files", "test1"}, File.separator);
            String file2 = ResourceUtils.getFile(file2Path).getPath();
            List<String> files = new ArrayList<>();
            files.add(file1);
            files.add(file2);
            String parentPath = (new File(file2)).getParent();
            zipPath = parentPath + File.separator + "test.zip";
            FileZip.zipMultiSpecFiles(files, zipPath);
            Assert.assertTrue((new File(zipPath)).exists());
        } finally {
            File file = new File(zipPath);
            if (file.exists()) {
                file.delete();
            }
        }
    }

    /**
     * 用例场景：压缩和解压文件夹
     * 前置条件：NA
     * 检查点：不报错
     */
    @Test
    public void test_zip_and_unzip() throws Exception {
        File file = new File("");
        StringBuilder sb = new StringBuilder();
        sb.append(file.getAbsolutePath()).append(File.separator);
        sb.append("src").append(File.separator);
        sb.append("test").append(File.separator);
        sb.append("resources").append(File.separator);
        File parent = new File(sb + "zipfiles");
        parent.mkdirs();
        try (FileOutputStream fos = new FileOutputStream(sb + "zipfiles" + File.separator + "test.txt")) {

        }

        // run the test
        FileZip.zipFile(sb + "zipfiles", sb + "zipfiles.zip");
        FileZip.unzipFile(sb + "zipfiles.zip", sb + "unzipfiles");
        Files.deleteIfExists(Paths.get(sb + "zipfiles.zip"));
        Files.deleteIfExists(Paths.get(sb + "unzipfiles" + File.separator + "zipfiles", "test.txt"));
        FileUtil.deleteDir(sb + "unzipfiles");
        FileUtil.deleteDir(sb + "zipfiles");
    }

    /**
     * 用例场景：压缩包深度过大
     * 前置条件：NA
     * 检查点：返回false
     */
    @Test
    public void test_check_when_depth_error() throws Exception {
        File file = new File("");
        StringBuilder sb = new StringBuilder();
        sb.append(file.getAbsolutePath()).append(File.separator);
        sb.append("src").append(File.separator);
        sb.append("test").append(File.separator);
        sb.append("resources").append(File.separator);
        File parent = new File(sb + "zipfiles");
        parent.mkdirs();
        File file1 = new File(sb + "zipfiles" + File.separator + "test1");
        file1.mkdirs();
        File file2 = new File(sb + "zipfiles" + File.separator + "test2");
        file2.mkdirs();
        try (FileOutputStream fos = new FileOutputStream(
            sb + "zipfiles" + File.separator + "test.txt");
            FileOutputStream fos2 = new FileOutputStream(
                sb + "zipfiles" + File.separator + "test2" + File.separator + "test.txt")) {

        }

        // run the test
        FileZip.zipFile(sb + "zipfiles", sb + "zipfiles.zip");
        FileZip fileZip = new FileZip();
        FileCheckRule fileCheckRule = getFileCheckRule(sb.toString());
        fileCheckRule.setMaxDepth(1);
        boolean result = fileZip.check(ClusterFileUtils.createMultipartFile(sb + "zipfiles.zip"), fileCheckRule);
        Files.deleteIfExists(Paths.get(sb + "zipfiles.zip"));
        FileUtil.deleteDir(sb + "unzipfiles");
        FileUtil.deleteDir(sb + "zipfiles");
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：压缩包包内文件过多
     * 前置条件：NA
     * 检查点：返回false
     */
    @Test
    public void test_check_when_entry_num_error() throws Exception {
        File file = new File("");
        StringBuilder sb = new StringBuilder();
        sb.append(file.getAbsolutePath()).append(File.separator);
        sb.append("src").append(File.separator);
        sb.append("test").append(File.separator);
        sb.append("resources").append(File.separator);
        File parent = new File(sb + "zipfiles");
        parent.mkdirs();
        File file1 = new File(sb + "zipfiles" + File.separator + "test1");
        file1.mkdirs();
        File file2 = new File(sb + "zipfiles" + File.separator + "test2");
        file2.mkdirs();
        try (FileOutputStream fos = new FileOutputStream(
            sb + "zipfiles" + File.separator + "test.txt");
            FileOutputStream fos2 = new FileOutputStream(
                sb + "zipfiles" + File.separator + "test2" + File.separator + "test.txt")) {

        }

        // run the test
        FileZip.zipFile(sb + "zipfiles", sb + "zipfiles.zip");
        FileZip fileZip = new FileZip();
        FileCheckRule fileCheckRule = getFileCheckRule(sb.toString());
        fileCheckRule.setMaxEntryNum(1);
        boolean result = fileZip.check(ClusterFileUtils.createMultipartFile(sb + "zipfiles.zip"), fileCheckRule);
        Files.deleteIfExists(Paths.get(sb + "zipfiles.zip"));
        FileUtil.deleteDir(sb + "unzipfiles");
        FileUtil.deleteDir(sb + "zipfiles");
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：压缩包包内文件大小过大
     * 前置条件：NA
     * 检查点：返回false
     */
    @Test
    public void test_check_when_entry_size_error() throws Exception {
        File file = new File("");
        StringBuilder sb = new StringBuilder();
        sb.append(file.getAbsolutePath()).append(File.separator);
        sb.append("src").append(File.separator);
        sb.append("test").append(File.separator);
        sb.append("resources").append(File.separator);
        File parent = new File(sb + "zipfiles");
        parent.mkdirs();
        File file1 = new File(sb + "zipfiles" + File.separator + "test1");
        file1.mkdirs();
        File file2 = new File(sb + "zipfiles" + File.separator + "test2");
        file2.mkdirs();
        try (FileOutputStream fos = new FileOutputStream(
            sb + "zipfiles" + File.separator + "test.txt");
            FileOutputStream fos2 = new FileOutputStream(
                sb + "zipfiles" + File.separator + "test2" + File.separator + "test.txt")) {
            byte[] content = "File content".getBytes();
            fos.write(content, 0, content.length);
        }

        // run the test
        FileZip.zipFile(sb + "zipfiles", sb + "zipfiles.zip");
        FileZip fileZip = new FileZip();
        FileCheckRule fileCheckRule = getFileCheckRule(sb.toString());
        fileCheckRule.setMaxEntrySize(10);
        boolean result = fileZip.check(ClusterFileUtils.createMultipartFile(sb + "zipfiles.zip"), fileCheckRule);
        Files.deleteIfExists(Paths.get(sb + "zipfiles.zip"));
        FileUtil.deleteDir(sb + "unzipfiles");
        FileUtil.deleteDir(sb + "zipfiles");
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：压缩包解压后大小过大
     * 前置条件：NA
     * 检查点：返回false
     */
    @Test
    public void test_check_when_total_size_error() throws Exception {
        File file = new File("");
        StringBuilder sb = new StringBuilder();
        sb.append(file.getAbsolutePath()).append(File.separator);
        sb.append("src").append(File.separator);
        sb.append("test").append(File.separator);
        sb.append("resources").append(File.separator);
        File parent = new File(sb + "zipfiles");
        parent.mkdirs();
        File file1 = new File(sb + "zipfiles" + File.separator + "test1");
        file1.mkdirs();
        File file2 = new File(sb + "zipfiles" + File.separator + "test2");
        file2.mkdirs();
        try (FileOutputStream fos = new FileOutputStream(
            sb + "zipfiles" + File.separator + "test.txt");
            FileOutputStream fos2 = new FileOutputStream(
                sb + "zipfiles" + File.separator + "test2" + File.separator + "test.txt")) {
            byte[] content = "File content".getBytes();
            fos.write(content, 0, content.length);
        }

        // run the test
        FileZip.zipFile(sb + "zipfiles", sb + "zipfiles.zip");
        FileZip fileZip = new FileZip();
        FileCheckRule fileCheckRule = getFileCheckRule(sb.toString());
        fileCheckRule.setMaxUnZipSize(10);
        boolean result = fileZip.check(ClusterFileUtils.createMultipartFile(sb + "zipfiles.zip"), fileCheckRule);
        Files.deleteIfExists(Paths.get(sb + "zipfiles.zip"));
        FileUtil.deleteDir(sb + "unzipfiles");
        FileUtil.deleteDir(sb + "zipfiles");
        Assert.assertFalse(result);
    }

    private FileCheckRule getFileCheckRule(String sb) {
        FileCheckRule fileCheckRule = new FileCheckRule();
        fileCheckRule.setMaxNameLength(512);
        fileCheckRule.setMaxSize(4L * 1024L * 1024L * 1024L);
        fileCheckRule.setMaxEntryNum(1024);
        fileCheckRule.setMaxEntrySize(4L * 1024L * 1024L * 1024L);
        fileCheckRule.setMaxDepth(1024);
        fileCheckRule.setTempPath(sb + "unzipfiles");
        fileCheckRule.setAllowedFormats(new HashSet<>());
        fileCheckRule.setMaxUnZipSize(4L * 1024L * 1024L * 1024L);
        fileCheckRule.setZipWhiteList(new HashSet<>());
        return fileCheckRule;
    }
}
