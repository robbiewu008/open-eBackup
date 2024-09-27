/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.util;

import openbackup.system.base.bean.FileValidateRule;
import openbackup.system.base.bean.ZipFileValidateRule;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.security.exterattack.ExterAttack;

import jodd.io.FileUtil;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.web.multipart.MultipartFile;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.math.BigDecimal;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipInputStream;

/**
 * 上传文件验证工具类
 *
 * @author hwx1144169
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-03-31
 */
@Slf4j
public class UploadFileValidateUtil {
    /**
     * 最大文件大小为2个G常量
     */
    private static final long MAX_FILE_LENGTH = 2L * 1024L * 1024L * 1024L;

    // zip压缩文件名后缀
    private static final String ZIP_FILE = ".zip";

    // GBK字符编码常量
    private static final String DEFAULT_CHARSET = "gbk";

    // 非法的文件名字符，包括":*?\"<>|"
    private static final String ILLEGAL_FILENAME_CHAR = ":*?\"<>|";

    // linux环境的非法文件名格式常量
    private static final String ILLEGAL_FILENAME_LINUX = "./";

    // windows环境的非法文件名格式常量
    private static final String ILLEGAL_FILENAME_WINDOWS = ".\\";

    // 压缩文件解压大小计数器存放在map中的key标识常量值
    private static final String DECOMPRESS_SIZE_COUNTER = "decompressSizeCounter";

    // zip文件头索引
    private static final int ZIP_HEADER_INDEX = 4;

    // zip文件头字节
    private static final byte[] ZIP_TYPE_HEADER = new byte[] {80, 75};

    // 1MB的大小
    private static final long MB = 1024L * 1024L;

    // linux环境路径分隔符
    private static final char LINUX_STYLE_PATH = '/';

    // windows环境路径分隔符
    private static final char WINDOWS_STYLE_PATH = '\\';

    // linux环境路径分隔符字符串
    private static final String PATH_DELIMITER = "/";

    // 文件名后缀的点符号常量
    private static final String FILE_SUFFIX_DOT = ".";

    private UploadFileValidateUtil() {
    }

    /**
     * 通用文件上传校验方法
     *
     * @param multipartFile 文件对象
     * @param fileValidateRule 校验规则对象
     */
    public static void validateCommonFile(MultipartFile multipartFile, FileValidateRule fileValidateRule) {
        validateCommonFile(multipartFile, fileValidateRule, false);
    }

    /**
     * 通用文件上传校验入口
     *
     * @param suffix 校验规则的文件后缀名
     * @param file 待校验的文件对象
     * @param isRequired  true:文件对象允许为null, false:文件对象不允许为null
     * @param fileSize 校验规则的文件大小
     * @return Boolean 校验结果
     */
    public static Boolean validateMultipartFile(String suffix, MultipartFile file, boolean isRequired, long fileSize) {
        FileValidateRule fileValidateRule = FileValidateRule.builder()
                .maxSize(fileSize)
                .suffix(suffix)
                .build();
        try {
            UploadFileValidateUtil.validateCommonFile(file, fileValidateRule, isRequired);
            return true;
        } catch (LegoCheckedException e) {
            log.error("common validate failed.", e);
            return false;
        }
    }

    /**
     * 通用文件上传校验方法
     *
     * @param multipartFile 文件对象
     * @param fileValidateRule 校验规则对象
     * @param canNull true:文件对象允许为null,false:文件对象不允许为null
     */
    public static void validateCommonFile(MultipartFile multipartFile, FileValidateRule fileValidateRule,
        boolean canNull) {
        // 验证文件对象是否为null
        validateFileNotNull(multipartFile, canNull);
        // 如果文件对象可以为null，并且对象确实为null，则return
        if (canNull && multipartFile == null) {
            return;
        }
        // 校验文件名长度
        validateFileNameLength(multipartFile.getOriginalFilename(), fileValidateRule.getNameMaxLength());
        // 校验文件名后缀
        validateFileNameSuffix(multipartFile.getOriginalFilename(), fileValidateRule.getSuffix());
        // 校验文件大小
        validateFileMaxSize(multipartFile.getSize(), fileValidateRule.getMaxSize());
        // 校验文件路径
        validateFilePath(fileValidateRule.getPath(), fileValidateRule.getPathRule());
    }

    /**
     * 验证文件对象是否为null
     *
     * @param multipartFile 文件对象
     * @param canNull true:文件对象允许为null,false:文件对象不允许为null
     */
    public static void validateFileNotNull(MultipartFile multipartFile, boolean canNull) {
        if (canNull) {
            return;
        }
        if (multipartFile == null) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "upload file is missing");
        }
    }

    /**
     * zip文件上传校验方法
     *
     * @param multipartFile 文件对象
     * @param zipFileValidateRule 压缩文件校验规则
     */
    public static void validateZipFile(MultipartFile multipartFile, ZipFileValidateRule zipFileValidateRule) {
        // 验证文件名长度
        validateFileNameLength(multipartFile.getOriginalFilename(), zipFileValidateRule.getNameMaxLength());
        // 校验压缩文件大小
        validateFileMaxSize(multipartFile.getSize(), zipFileValidateRule.getMaxSize());
        // 1、将上传文件写到本地，并返回本地路径
        String tempPathFinal = getTempPathFinal(zipFileValidateRule);
        String fileLocalPath = getFileLocalPath(multipartFile, tempPathFinal);
        // 解压文件临时存放目录
        String unzipTempPath = zipFileValidateRule.getUnzipTempPath() + UUID.randomUUID();
        try {
            // 2、校验zip文件是否受损
            validateIntegrity(fileLocalPath);
            // 3、验证是否是zip文件
            validateFileHeader(fileLocalPath);
            // 4、压缩炸弹校验：解压zip压缩包，并检验zip文件的安全性，拦截zip炸弹
            validateZipSafety(fileLocalPath, unzipTempPath, zipFileValidateRule.getFileMaxNum(),
                zipFileValidateRule.getMaxDecompressSize(), zipFileValidateRule.getSubFileList());
        } finally {
            try {
                FileUtil.cleanDir(tempPathFinal);
                log.info("success to delete input zip.");
                if (StringUtils.isNotEmpty(unzipTempPath)) {
                    FileUtil.cleanDir(unzipTempPath);
                    log.info("success to delete unzip temp file");
                }
            } catch (IOException e) {
                log.error("The uploaded file was not deleted normally.");
            }
        }
    }

    private static String getTempPathFinal(ZipFileValidateRule zipFileValidateRule) {
        if (StringUtils.isBlank(zipFileValidateRule.getTempPath())) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "tempPath is null");
        }
        return zipFileValidateRule.getTempPath() + UUID.randomUUID();
    }

    /**
     * 获取压缩文件上传到本地的临时文件完整路径
     *
     * @param multipartFile 文件对象
     * @param tempPath 压缩文件临时存放路径
     * @return fileLocalPath 本地的临时文件完整路径
     */
    public static String getFileLocalPath(MultipartFile multipartFile, String tempPath) {
        if (StringUtils.isEmpty(tempPath)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "tempPath is null");
        }
        // 上传压缩文件到 tempPath 目录
        String localFileName = writeUploadFileToLocal(multipartFile, tempPath);
        return tempPath + File.separator + localFileName;
    }

    /**
     * 校验文件名长度
     *
     * @param originalFilename 原始文件名
     * @param fileNameMaxLength 文件名限制长度
     */
    public static void validateFileNameLength(String originalFilename, int fileNameMaxLength) {
        // 若fileValidateRule参数中没有文件名最大长度限制，则默认用512
        int nameMaxLength = fileNameMaxLength == IsmNumberConstant.ZERO ? 512 : fileNameMaxLength;
        String uploadFileName = getUploadFileName(originalFilename);
        if (uploadFileName.length() > nameMaxLength) {
            log.error("upload file fail, file name is too long. name length : {}.", uploadFileName.length());
            throw new LegoCheckedException(CommonErrorCode.FILE_NAME_LENGTH_VALIDATE_FAILED,
                new String[] {String.valueOf(nameMaxLength)}, "file name is too long.");
        }
    }

    /**
     * 校验文件名后缀是否符合规则
     *
     * @param originalFilename 文件名
     * @param suffix 文件校验规则对象
     */
    public static void validateFileNameSuffix(String originalFilename, String suffix) {
        if (!StringUtils.isNotEmpty(suffix)) {
            log.warn("no suffix rule.");
            return;
        }
        boolean hasDot = originalFilename.contains(FILE_SUFFIX_DOT) && !originalFilename.startsWith(FILE_SUFFIX_DOT);
        String uploadFileName = getUploadFileName(originalFilename);
        if (!hasDot || !uploadFileName.endsWith(suffix)) {
            log.error("upload file name suffix error.");
            throw new LegoCheckedException(CommonErrorCode.FILE_NAME_SUFFIX_VALIDATE_FAILED,
                "upload file name suffix error.");
        }
    }

    /**
     * 验证文件大小
     *
     * @param fileSize 文件真实大小
     * @param fileMaxSize 文件限制的最大大小
     */
    public static void validateFileMaxSize(long fileSize, long fileMaxSize) {
        // 若fileValidateRule没有文件大小限制，则默认最大限制为2G
        long maxSize = fileMaxSize == IsmNumberConstant.ZERO ? MAX_FILE_LENGTH : fileMaxSize;
        if (fileSize == 0) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "upload file is empty.");
        }
        if (fileSize > maxSize) {
            throw new LegoCheckedException(CommonErrorCode.FILE_SIZE_VALIDATE_FAILED,
                new String[] {convertFileSizeToMB(maxSize)}, "upload file is too large.");
        }
    }

    /**
     * 将文件大小单位转换为MB
     *
     * @param fileMaxSize 以B为单位的文件大小
     * @return string 带MB格式的文件大小
     */
    private static String convertFileSizeToMB(long fileMaxSize) {
        BigDecimal bigDecimal = new BigDecimal(fileMaxSize);
        return bigDecimal.divide(new BigDecimal(MB), 3, BigDecimal.ROUND_HALF_UP) + "MB";
    }

    /**
     * 路径规则校验
     *
     * @param filePath 文件路径
     * @param filePathRule 路径规则正则表达式
     */
    public static void validateFilePath(String filePath, String filePathRule) {
        if (!StringUtils.isNotEmpty(filePathRule)) {
            log.warn("no path rule.");
            return;
        }
        if (!Pattern.compile(filePathRule).matcher(filePath).matches()) {
            log.error("file path validate failure.");
            throw new LegoCheckedException(CommonErrorCode.FILE_PATH_VALIDATE_FAILED, "file path validate failure.");
        }
    }

    /**
     * 获取上传文件的真实文件名
     *
     * @param originalFilename 文件名称
     * @return String 获取文件真实名称
     */
    private static String getUploadFileName(String originalFilename) {
        String fileName = originalFilename;
        // Check for Unix-style path
        int unixSep = fileName.lastIndexOf(LINUX_STYLE_PATH);
        // Check for Windows-style path
        int winSep = fileName.lastIndexOf(WINDOWS_STYLE_PATH);
        // Cut off at latest possible point
        int pos = Math.max(winSep, unixSep);
        if (pos != -1) {
            // Any sort of path separator found...
            fileName = fileName.substring(pos + 1);
        }
        return fileName;
    }

    /**
     * 检查文件名称是否合法(防止解压绕过等）
     *
     * @param fileName 文件名称
     */
    public static void verifyFileName(String fileName) {
        List<Integer> collect = ILLEGAL_FILENAME_CHAR.chars().boxed().collect(Collectors.toList());
        fileName.chars().boxed().forEach(intChar -> {
            if (collect.contains(intChar)) {
                throw new LegoCheckedException(CommonErrorCode.FILE_NAME_SECURITY_VALIDATE_FAILED,
                    "file name illegal.");
            }
        });
        // 非法文件名
        if (fileName.contains(ILLEGAL_FILENAME_LINUX) || fileName.contains(ILLEGAL_FILENAME_WINDOWS)) {
            throw new LegoCheckedException(CommonErrorCode.FILE_NAME_SECURITY_VALIDATE_FAILED, "file name illegal.");
        }
    }

    /**
     * 解压zip压缩包，并检验zip文件的安全性，拦截zip炸弹
     *
     * @param fileLocalPath 本地文件路径
     * @param unzipTempPath 压缩文件解压临时路径
     * @param fileMaxNum 解压文件允许的最大文件数量
     * @param maxDecompressSize 解压文件允许的最大大小
     * @param subFileList 压缩文件中的子文件列表
     */
    @ExterAttack
    public static void validateZipSafety(String fileLocalPath, String unzipTempPath, long fileMaxNum,
        long maxDecompressSize, List<String> subFileList) {
        // 压缩文件校验必须指定解压文件的临时存放路径
        if (StringUtils.isEmpty(unzipTempPath)) {
            log.warn("unzipTempPath is null");
            return;
        }
        // 定义解压文件大小计数器
        Map<String, Long> decompressSizeMap = new HashMap<>();
        decompressSizeMap.put(DECOMPRESS_SIZE_COUNTER, 0L);
        // 解压过程中存放文件目录的集合
        List<File> unZipTempDirList = new ArrayList<>();
        // 压缩文件中子文件map
        Map<String, File> inputFileMap = new HashMap<>();
        try (FileInputStream inputStream = FileUtils.openInputStream(new File(fileLocalPath));
            BufferedInputStream bufferedInputStream = new BufferedInputStream(inputStream);
            ZipInputStream zip = new ZipInputStream(bufferedInputStream, Charset.forName(DEFAULT_CHARSET))) {
            ZipEntry zipEntry;
            long entryCounter = 0L;
            while ((zipEntry = zip.getNextEntry()) != null) {
                String fileName = new String(zipEntry.getName().getBytes(), StandardCharsets.UTF_8);
                // 文件名安全检查，防解压路径绕过
                verifyFileName(fileName);
                if (zipEntry.isDirectory()) {
                    createDir(unzipTempPath, fileName, unZipTempDirList);
                    continue;
                }
                File file = readPerEntry(zip, unzipTempPath, fileName, maxDecompressSize, decompressSizeMap);
                entryCounter++;
                // 校验压缩文件中的数量是否符合要求
                checkUnzipFileNum(entryCounter, fileMaxNum);
                // 保存已解压的文件信息到map
                inputFileMap.put(formatName(fileName), file);
            }
            // 检查上传文件是否齐全
            validateSubFileNum(inputFileMap, subFileList);
        } catch (IOException e) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "The zip file stream parsing failed.",
                ExceptionUtil.getErrorMessage(e));
        } finally {
            // 删除解压的文件
            deleteUnZipFilesAndDirs(unZipTempDirList);
        }
    }

    /**
     * 检查上传文件是否齐全
     *
     * @param inputFileMap 解压后的文件map
     * @param subFileList 已知的压缩文件子文件列表
     */
    private static void validateSubFileNum(Map<String, File> inputFileMap, List<String> subFileList) {
        if (CollectionUtils.isEmpty(subFileList)) {
            log.warn("subFileList rule is null");
            return;
        }
        if (!inputFileMap.keySet().containsAll(subFileList)) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "The zip file stream parsing failed.");
        }
    }

    /**
     * 创建目录
     *
     * @param unzipTempPath 解压文件临时存放路径
     * @param fileName 文件名
     * @param unZipTempDirList 解压临时文件路径存放列表
     * @throws IOException IO异常
     */
    private static void createDir(String unzipTempPath, String fileName, List<File> unZipTempDirList)
        throws IOException {
        File dir = new File(unzipTempPath + File.separator + fileName);
        if (dir.mkdirs()) {
            log.info("success to unzip dir path.");
            unZipTempDirList.add(dir);
        }
    }

    /**
     * 读取压缩文件
     *
     * @param zip zip文件流
     * @param unzipTempPath 解压的目标目录
     * @param fileName 文件名
     * @param maxDecompressSize 解压文件解压后最大允许大小
     * @param decompressSizeMap 解压文件计数器
     * @return File 解压后的文件对象
     */
    private static File readPerEntry(InputStream zip, String unzipTempPath, String fileName, long maxDecompressSize,
        Map<String, Long> decompressSizeMap) {
        try (FileOutputStream fileOutputStream = FileUtils.openOutputStream(
            new File(unzipTempPath + File.separator + fileName));
            BufferedOutputStream os = new BufferedOutputStream(fileOutputStream)) {
            File targetDir = new File(unzipTempPath);
            if (!targetDir.exists()) {
                targetDir.mkdirs();
            }
            byte[] temp = new byte[LegoNumberConstant.BYTE_SIZE];
            int len;
            while ((len = zip.read(temp)) != -1) {
                os.write(temp, 0, len);
                decompressSizeMap.put(DECOMPRESS_SIZE_COUNTER, decompressSizeMap.get(DECOMPRESS_SIZE_COUNTER) + len);
                // 检查解压后文件总大小是否超过设定的解压后最大文件大小标准值
                checkUnzipTotalSize(decompressSizeMap.get(DECOMPRESS_SIZE_COUNTER), maxDecompressSize);
            }
        } catch (IOException e) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "Error to close the upload file stream.",
                ExceptionUtil.getErrorMessage(e));
        }
        log.info("success to unzip file.");
        return new File(unzipTempPath + File.separator + fileName);
    }

    /**
     * 检查解压文件数量是否超过规定的最大值
     *
     * @param entriesNum 文件数量
     * @param maxEntriesNum 允许的最大的文件数量
     */
    private static void checkUnzipFileNum(long entriesNum, long maxEntriesNum) {
        if (maxEntriesNum > IsmNumberConstant.ZERO && entriesNum > maxEntriesNum) {
            log.error("uncompress result num is too many than max num: {}.", maxEntriesNum);
            throw new LegoCheckedException(CommonErrorCode.FILE_SIZE_VALIDATE_FAILED,
                new String[] {String.valueOf(maxEntriesNum)}, "Uncompress result num is too many than max num.");
        }
    }

    /**
     * 边解压，边校验解压文件的大小是否超过最大设置大小
     *
     * @param decompressSize 解压文件的当前大小
     * @param maxDecompressSize 规则限定的解压文件最大大小
     */
    private static void checkUnzipTotalSize(long decompressSize, long maxDecompressSize) {
        long maxSize = maxDecompressSize == IsmNumberConstant.ZERO
            ? MAX_FILE_LENGTH
            : maxDecompressSize;
        if (decompressSize > maxSize) {
            log.error("uncompress result size: {} more than max size: {}.", decompressSize, maxDecompressSize);
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "Uncompress result size ({}) more than max size.");
        }
    }

    /**
     * 删除解压的相关文件和文件夹
     *
     * @param unZipTempDirList 解压临时目录列表
     */
    private static void deleteUnZipFilesAndDirs(List<File> unZipTempDirList) {
        for (File dir : unZipTempDirList) {
            if (!dir.exists()) {
                log.warn("the unzip file is not exist already.");
                continue;
            }
            log.info("ready to delete local file name.");
            try {
                FileUtil.deleteDir(dir);
                log.info("success to delete local file.");
            } catch (IOException e) {
                log.error("error to delete the unzip file.");
            }
        }
    }

    /**
     * 将上传的文件写入本地
     *
     * @param file 备份文件
     * @param tempPath 本地临时存放目录
     * @return 本地文件名称
     */
    private static String writeUploadFileToLocal(MultipartFile file, String tempPath) {
        String targetFileName = UUID.randomUUID() + ZIP_FILE;
        try {
            writeToLocal(file.getInputStream(), tempPath, targetFileName);
            log.info("success to save input file to local.");
        } catch (IOException e) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "Error to get source file input stream.",
                ExceptionUtil.getErrorMessage(e));
        }
        return targetFileName;
    }

    /**
     * 将MultipartFile写到本地磁盘
     *
     * @param fileInputStream 上传的文件流
     * @param targetPath 目标写入路径
     * @param targetFileName 目标写入文件名称
     */
    private static void writeToLocal(InputStream fileInputStream, String targetPath, String targetFileName) {
        File targetDir = new File(targetPath);
        if (!targetDir.exists()) {
            try {
                FileUtil.mkdirs(targetDir);
            } catch (IOException e) {
                throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "Error to create temp directory.",
                    ExceptionUtil.getErrorMessage(e));
            }
        }
        try (InputStream inputStream = fileInputStream;
            OutputStream os = FileUtils.openOutputStream(new File(targetPath + File.separator + targetFileName))) {
            byte[] temp = new byte[LegoNumberConstant.BYTE_SIZE];
            int len;
            while ((len = inputStream.read(temp)) != -1) {
                os.write(temp, 0, len);
            }
            os.flush();
        } catch (IOException e) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "Error to close the upload file stream.",
                ExceptionUtil.getErrorMessage(e));
        }
    }

    /**
     * 验证是否为zip压缩文件
     *
     * @param fileLocalPath 文件本地路径
     */
    @ExterAttack
    public static void validateFileHeader(String fileLocalPath) {
        boolean isValidHeader;
        File file = new File(fileLocalPath);
        try (FileInputStream fileInputStream = FileUtils.openInputStream(file)) {
            byte[] headerBytes = new byte[ZIP_HEADER_INDEX];
            fileInputStream.read(headerBytes);
            isValidHeader = headerBytes[0] == ZIP_TYPE_HEADER[0] && headerBytes[1] == ZIP_TYPE_HEADER[1];
            if (!isValidHeader) {
                log.error("fail to upload file, file is not a zip file. head bytes: {}", Arrays.toString(headerBytes));
                throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
                    "The uploaded file is not in .zip format.");
            }
        } catch (IOException e) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "Error to find local input zip file.");
        }
    }

    /**
     * 格式化文件名
     *
     * @param fileName 文件名
     * @return 格式化后的文件名
     */
    private static String formatName(String fileName) {
        if (!fileName.contains(PATH_DELIMITER)) {
            return fileName;
        }
        return Paths.get(fileName).getFileName().toString();
    }

    /**
     * 校验zip文件是否受损
     *
     * @param fileLocalPath zip文件完整路径
     */
    public static void validateIntegrity(String fileLocalPath) {
        try (ZipFile zipfile = new ZipFile(new File(fileLocalPath))) {
            log.info("validate zip integrity success.");
        } catch (IOException e) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "zip file is corrupted.");
        }
    }
}
