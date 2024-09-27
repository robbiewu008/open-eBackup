package openbackup.system.base.util;

/**
 * @author hwx1144169
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-04-06
 */

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.bean.FileValidateRule;
import openbackup.system.base.bean.ZipFileValidateRule;
import openbackup.system.base.util.UploadFileValidateUtil;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.springframework.mock.web.MockMultipartFile;
import org.springframework.web.multipart.MultipartFile;

public class UploadFileValidateUtilTest {

    /**
     * 预期异常
     */
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    /**
     * 文件名长度超过规定长度，验证失败
     */
    @Test
    public void test_file_name_length_validate_failed() {
        expectedException.expect(LegoCheckedException.class);
        String originalFilename = "test.txt";
        int fileNameMaxLength = 7;
        UploadFileValidateUtil.validateFileNameLength(originalFilename, fileNameMaxLength);
    }

    /**
     * 文件名后缀不符合规则，验证失败
     */
    @Test
    public void test_file_name_suffix_validate_failed() {
        expectedException.expect(LegoCheckedException.class);
        String originalFilename = "test.txt";
        String suffix = ".zip";
        UploadFileValidateUtil.validateFileNameSuffix(originalFilename, suffix);
    }

    /**
     * 文件大小超过规定最大大小，验证失败
     */
    @Test
    public void test_file_size_validate_failed() {
        expectedException.expect(LegoCheckedException.class);
        long fileSize = 3L * 1024L * 1024L * 1024L;
        long fileMaxSize = 2L * 1024L * 1024L * 1024L;
        UploadFileValidateUtil.validateFileMaxSize(fileSize, fileMaxSize);
    }

    /**
     * 文件路径不符合要求格式，验证失败
     */
    @Test
    public void test_file_path_validate_failed() {
        expectedException.expect(LegoCheckedException.class);
        String filePath = "/opt/OceanProtect";
        String filePathRule = "^[a-zA-Z]:\\\\";
        UploadFileValidateUtil.validateFilePath(filePath, filePathRule);
    }

    @Test
    public void test_validate_upload_file() {
        MockMultipartFile mockMultipartFile = new MockMultipartFile("xx.zip", "xx.zip", "test_content", new byte[1024]);
        FileValidateRule fileValidateRule = FileValidateRule.builder().suffix(".zip").build();
        UploadFileValidateUtil.validateCommonFile(mockMultipartFile, fileValidateRule);
    }

    /**
     * 用例名称：检查通用文件检查入口
     * 前置条件：X8000运行正常
     * check点：分别断言校验失败和校验成功
     */
    @Test
    public void test_common_validate_upload_file() {
        MockMultipartFile file = new MockMultipartFile("xx.zip", "xx.zip", "test_content", new byte[1024]);
        Assert.assertTrue(UploadFileValidateUtil.validateMultipartFile("zip", file, true, 1048576L));
        Assert.assertFalse(UploadFileValidateUtil.validateMultipartFile("txt", file, true, 1048576L));
    }

    /**
     * 文件名安全性验证方法
     */
    @Test
    public void test_upload_file_name_verify_success() {
        String originalFilename = "../test.zip";
        Assert.assertThrows(LegoCheckedException.class, () -> UploadFileValidateUtil.verifyFileName(originalFilename));
    }

    @Test
    public void test_validate_zip_safety() {
        byte[] bytes = new byte[2048];
        MultipartFile file = new MockMultipartFile("zip_test_example.zip", "zip_test_example.zip", "", bytes);
        ZipFileValidateRule zipFileValidateRule = ZipFileValidateRule.builder()
            .tempPath("/home/zip_test")
            .unzipTempPath("/home/zip_test")
            .fileMaxNum(10)
            .maxDecompressSize(1024)
            .build();
        Assert.assertThrows(LegoCheckedException.class,
            () -> UploadFileValidateUtil.validateZipFile(file, zipFileValidateRule));
    }

    @Test
    public void test_write_upload_file_to_local() {
        byte[] bytes = new byte[1024];
        MultipartFile file = new MockMultipartFile("zip_test_example.zip", "zip_test_example.zip", "", bytes);
        String tempPath = "";
        Assert.assertThrows(LegoCheckedException.class,
            () -> UploadFileValidateUtil.getFileLocalPath(file, tempPath));
    }

    @Test
    public void test_validate_file_header() {
        String fileLocalPath;
        byte[] bytes = new byte[1024];
        MultipartFile file = new MockMultipartFile("zip_test_example.zip", "zip_test_example.zip", "", bytes);
        String unzipTempPath = "/home/zip_test";
        fileLocalPath = UploadFileValidateUtil.getFileLocalPath(file, unzipTempPath);
        Assert.assertThrows(LegoCheckedException.class, () -> UploadFileValidateUtil.validateFileHeader(fileLocalPath));
    }

    @Test
    public void test_file_integrity(){
        String fileLocalPath;
        byte[] bytes = new byte[1024];
        MultipartFile file = new MockMultipartFile("zip_test_example.zip", "zip_test_example.zip", "", bytes);
        String unzipTempPath = "/home/zip_test";
        fileLocalPath = UploadFileValidateUtil.getFileLocalPath(file, unzipTempPath);
        Assert.assertThrows(LegoCheckedException.class, () -> UploadFileValidateUtil.validateIntegrity(fileLocalPath));
    }

    @Test
    public void file_is_required(){
        MultipartFile file = null;
        boolean isRequired = false;
        Assert.assertThrows(LegoCheckedException.class, () -> UploadFileValidateUtil.validateFileNotNull(file, isRequired));
    }

    /**
     * 用例名称：检查文件可以为null，并且文件真的为null
     * 前置条件：文件null，并且can null
     * check点：不报错
     */
    @Test
    public void validate_file_success_when_can_null() {
        UploadFileValidateUtil.validateCommonFile(null, null, true);
    }
}
