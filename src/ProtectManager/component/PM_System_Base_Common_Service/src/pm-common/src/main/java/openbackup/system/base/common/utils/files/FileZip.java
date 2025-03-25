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
package openbackup.system.base.common.utils.files;

import openbackup.system.base.bean.FileCheckRule;
import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.util.IdUtil;

import net.lingala.zip4j.ZipFile;
import net.lingala.zip4j.model.ZipParameters;
import net.lingala.zip4j.model.enums.CompressionMethod;
import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry;
import org.apache.commons.compress.archivers.zip.ZipArchiveInputStream;
import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.web.multipart.MultipartFile;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

/**
 * FileZip
 *
 */
public class FileZip implements FileCheckInterface {
    private static final Logger logger = LoggerFactory.getLogger(FileZip.class);

    private static final long GB_BYTE_NUM = 1024L * 1024L * 1024L;

    private static final String ZIP_POSTFIX = ".zip";

    /**
     * 创建压缩包
     * 建议使用该压缩方法
     *
     * @param sourceFilePath 文件源路径
     * @param zipFilePath 压缩包路径
     */
    public static void zipFile(String sourceFilePath, String zipFilePath) {
        zipMultiFiles(Collections.singletonList(sourceFilePath), zipFilePath);
    }

    /**
     * 多个目录打包
     * 建议使用该压缩方法
     *
     * @param sourceFilePathList 源文件路径列表
     * @param zipFilePath 压缩包路径
     */
    public static void zipMultiFiles(List<String> sourceFilePathList, String zipFilePath) {
        try (FileOutputStream fos = new FileOutputStream(zipFilePath); ZipOutputStream zos = new ZipOutputStream(fos)) {
            for (String sourceFilePath : sourceFilePathList) {
                // 遍历文件夹并递归添加文件到压缩包
                File folderToCompress = new File(sourceFilePath);
                if (folderToCompress.exists()) {
                    // isRoot用来判断是否是根目录
                    addFilesToZip(folderToCompress, folderToCompress.getName(), zos, true);
                }
            }
        } catch (IOException e) {
            logger.error("package multi file to zip error", ExceptionUtil.getErrorMessage(e));
            File file = new File(zipFilePath);
            FileUtil.deleteDir(file);
            throw new LegoCheckedException("package multi file to zip error.");
        }
    }

    /**
     * 解压压缩包
     * 建议使用该解压方法
     *
     * @param zipFilePath zip包路径
     * @param unzipPath 解压目标路径
     */
    public static void unzipFile(String zipFilePath, String unzipPath) {
        try (FileInputStream fis = new FileInputStream(zipFilePath);
            ZipArchiveInputStream zis = new ZipArchiveInputStream(fis)) {
            ZipArchiveEntry entry;
            long totalSize = 0L;
            long entryNum = 0L;
            while ((entry = zis.getNextZipEntry()) != null) {
                entryNum++;
                if (entryNum >= LegoNumberConstant.THROUND_TWENTY_FOUR) {
                    logger.error("zip file entry is too many, num: {}", entryNum);
                    throw new LegoCheckedException("zip file entry is too many, num.");
                }

                // 写文件
                totalSize += writeToFile(zis, entry, unzipPath);

                if (totalSize >= GB_BYTE_NUM * 4L) {
                    logger.error("unzip fail, total size is too big, name: {}, size: {}", entry.getName(), totalSize);
                    throw new LegoCheckedException("zip file total size is too big.");
                }
            }
        } catch (IOException | LegoCheckedException e) {
            logger.error("unzip error", ExceptionUtil.getErrorMessage(e));
            File file = new File(unzipPath);
            FileUtil.deleteDir(file);
            throw LegoCheckedException.cast(e);
        }
    }

    /**
     * 压缩文件<br>
     * 1.支持场景1--选择一个报表插件目录打成ZIP压缩包
     *
     * @param inputFilePath 需要压缩的文件夹或者文件路径
     * @param zipFilePath 保存的压缩包文件路径
     * @param baseName 保存的压缩包文件名
     */
    public static void zip(String inputFilePath, String zipFilePath, String baseName) {
        try (ZipOutputStream zOut = new ZipOutputStream(
            FileUtils.openOutputStream(new File(new String(zipFilePath.getBytes("gb2312"), "gb2312"))));
            BufferedOutputStream bos = new BufferedOutputStream(zOut)) {
            // 创建文件输出对象out,提示:注意中文支持
            logger.info("zip file begin, code: {}", ErrorCodeConstant.SYSTEM_COMMON_BASE);
            zip(zOut, bos, new File(inputFilePath), baseName);
            logger.info("zip file end, code: {}", ErrorCodeConstant.SYSTEM_COMMON_BASE);
        } catch (UnsupportedEncodingException e) {
            logger.error("code String with gb2312 failed.", ExceptionUtil.getErrorMessage(e));
        } catch (Exception e) {
            logger.error("Exception: ", ExceptionUtil.getErrorMessage(e));
        }
    }

    /**
     * 将多个文件打包压缩成一个文件
     *
     * @param inputFilePaths 需要压缩的文件列表
     * @param zipFilePath 保存的压缩包文件路径
     */
    public static void zipMultiSpecFiles(List<String> inputFilePaths, String zipFilePath) {
        try (ZipOutputStream zOut = new ZipOutputStream(
            FileUtils.openOutputStream(new File(new String(zipFilePath.getBytes("gb2312"), "gb2312"))));
            BufferedOutputStream bos = new BufferedOutputStream(zOut)) {
            logger.info("zip file begin, code: {}", ErrorCodeConstant.SYSTEM_COMMON_BASE);
            for (String filePath : inputFilePaths) {
                File tmpFile = new File(filePath);
                logger.info("zip file: name: {}", tmpFile.getName());
                zip(zOut, bos, tmpFile, tmpFile.getName());
                logger.info("zip file end, code: {}", ErrorCodeConstant.SYSTEM_COMMON_BASE);
            }
        } catch (UnsupportedEncodingException e) {
            logger.error("code String with gb2312 failed.", ExceptionUtil.getErrorMessage(e));
        } catch (Exception e) {
            logger.error("Exception: ", ExceptionUtil.getErrorMessage(e));
        }
    }

    /**
     * zip解压
     *
     * @param inputFile 待解压文件名
     * @param outputDir 解压路径
     */
    @ExterAttack
    public static void unzip(String inputFile, String outputDir) {
        File srcFile = new File(inputFile);
        try (ZipInputStream zIn = new ZipInputStream(FileUtils.openInputStream(srcFile))) {
            ZipEntry entry = zIn.getNextEntry();
            if (entry == null) {
                logger.error("entry == null");
                return;
            }
            long entryNum = 0L;
            long totalSize = 0L;
            do {
                entryNum++;
                if (!entry.isDirectory()) {
                    totalSize = unzipEntryToFile(outputDir, zIn, entry, totalSize);
                }
                if (entryNum >= LegoNumberConstant.THROUND_TWENTY_FOUR) {
                    logger.error("zip file entry is too many, num : {}", entryNum);
                    throw new LegoCheckedException("zip file entry is too many, num.");
                }
                entry = zIn.getNextEntry();
            } while (entry != null);
        } catch (IOException e) {
            logger.error("Exception: ", ExceptionUtil.getErrorMessage(e));
        }
    }

    /**
     * 将指定文件压缩到指定目录中
     *
     * @param paths 压缩文件目录
     * @param targetPath 目标目录位置
     */
    public static void zipFolders(List<String> paths, String targetPath) {
        logger.debug("paths:{}", paths);
        ZipParameters zipParameters = new ZipParameters();
        zipParameters.setCompressionMethod(CompressionMethod.DEFLATE);
        try (ZipFile targetFile = new ZipFile(targetPath)) {
            for (String path : paths) {
                File file = new File(path);
                if (!file.exists()) {
                    return;
                }
                if (file.isFile()) {
                    targetFile.addFile(file);
                } else {
                    targetFile.addFolder(file);
                }
            }
        } catch (IOException e) {
            logger.error("zip file error", ExceptionUtil.getErrorMessage(e));
        }
    }

    private static long unzipEntryToFile(String outputDir, ZipInputStream zIn, ZipEntry entry, long totalSize)
        throws IOException {
        if (entry.getName().contains(FileUtil.DOUBLE_DOT_SLASH)) {
            logger.error("unzip file fail, file name unsafe, entry name : {}.", entry.getName());
            throw new LegoCheckedException("unzip file fail, file name unsafe.");
        }
        File file = new File(outputDir, entry.getName());
        if (!file.exists()) {
            if (file.getParent() != null) {
                new File(file.getParent()).mkdirs();
            }
        }
        long calTotalSize;
        try (BufferedOutputStream bos = new BufferedOutputStream(FileUtils.openOutputStream(file))) {
            byte[] buf = new byte[LegoNumberConstant.BYTE_SIZE * 4];
            int len = zIn.read(buf);
            long entrySize = len;
            calTotalSize = totalSize;
            while (len != LegoNumberConstant.NEGATIVE_ONE) {
                entrySize += len;
                calTotalSize += len;
                if (entrySize >= GB_BYTE_NUM * 2L) {
                    logger.error("unzip fail, zip entry is too big, name: {}, size: {}", entry.getName(), entrySize);
                    throw new LegoCheckedException("zip file entry size is too big.");
                }
                if (calTotalSize >= GB_BYTE_NUM * 4L) {
                    logger.error("unzip fail, total size is too big, name: {}, size: {}", entry.getName(), totalSize);
                    throw new LegoCheckedException("zip file total size is too big.");
                }
                bos.write(buf, 0, len);
                len = zIn.read(buf);
            }
        }
        return calTotalSize;
    }

    private static void zip(ZipOutputStream zOut, BufferedOutputStream bos, File file, String base) {
        String fileName = base;
        try {
            logger.info("zip file--> : {}", file.getName());
            if (file.isDirectory()) {
                File[] listFiles = file.listFiles();
                if (listFiles != null) {
                    zOut.putNextEntry(new ZipEntry(fileName + File.separator));
                    fileName = fileName.length() == 0 ? "" : fileName + File.separator;
                    for (File listFile : listFiles) {
                        zip(zOut, bos, listFile, fileName + listFile.getName());
                    }
                }
            } else {
                if ("".equals(fileName)) {
                    fileName = file.getName();
                }
                zOut.putNextEntry(new ZipEntry(fileName));
                try (BufferedInputStream in = new BufferedInputStream(Files.newInputStream(file.toPath()))) {
                    int len;
                    byte[] temp = new byte[LegoNumberConstant.BYTE_SIZE];
                    while ((len = in.read(temp)) != LegoNumberConstant.NEGATIVE_ONE) {
                        bos.write(temp, 0, len);
                    }
                }
                bos.flush();
            }
        } catch (Exception e) {
            logger.error("Exception: ", ExceptionUtil.getErrorMessage(e));
        }
    }

    /**
     * 从multipartFile获取文件并解压到指定位置, 只清理zip文件。异常由调用方处理，临时文件由调用方清理，文件由调用方校验
     *
     * @param multipartFile 待处理的multipartFile对象
     * @param basePath 文件路径
     * @param fileName 文件名
     * @param tmpPath 临时目录
     * @throws IOException IOException
     */
    public static void unzipMultipartFileToTmp(MultipartFile multipartFile, String basePath, String fileName,
        String tmpPath) throws IOException {
        File zipFile = null;
        try {
            String zipFilePath = String.valueOf(Paths.get(basePath, fileName + ZIP_POSTFIX));
            zipFile = new File(zipFilePath);
            String unzipTmpPath = Paths.get(basePath, tmpPath).toString();
            FileUtils.copyInputStreamToFile(multipartFile.getInputStream(), zipFile);
            FileZip.unzip(zipFilePath, unzipTmpPath);
        } finally {
            if (Objects.nonNull(zipFile) && zipFile.exists()) {
                zipFile.delete();
            }
        }
    }

    /**
     * 从multipartFile获取文件并解压到指定位置
     *
     * @param multipartFile 待处理的multipartFile对象
     * @param basePath 文件路径
     * @param fileName 文件名
     * @param tmpPath 临时目录
     */
    public static void unzipMultipartFile(MultipartFile multipartFile, String basePath, String fileName,
        String tmpPath) {
        logger.info("unzip multipart file start");
        String zipFilePath = String.valueOf(Paths.get(basePath, fileName + ZIP_POSTFIX));
        File zipFile = new File(zipFilePath);
        String unzipTmpPath = Paths.get(basePath, tmpPath).toString();
        File unzipTmpFile = new File(unzipTmpPath);
        try {
            FileUtils.copyInputStreamToFile(multipartFile.getInputStream(), zipFile);
            FileZip.unzip(zipFilePath, unzipTmpPath);
            FileUtils.copyDirectory(new File(String.valueOf(Paths.get(unzipTmpPath, fileName))),
                new File(String.valueOf(Paths.get(basePath, fileName))));
            logger.info("unzip multipart file success");
        } catch (IOException e) {
            logger.error("unzip multipart file failed.", ExceptionUtil.getErrorMessage(e));
            throw LegoCheckedException.cast(e);
        } finally {
            logger.info("delete tmp file start");
            // 删除临时文件
            if (zipFile.exists()) {
                zipFile.delete();
            }
            if (unzipTmpFile.exists()) {
                deleteDir(unzipTmpFile);
            }
            logger.info("delete tmp file success");
        }
    }

    private static void deleteDir(File dir) {
        if (dir.isDirectory()) {
            String[] children = dir.list();
            for (int i = 0; i < children.length; i++) {
                deleteDir(new File(dir, children[i]));
            }
        }
        dir.delete();
    }

    private static void addFilesToZip(File file, String parentFolderName, ZipOutputStream zos, boolean isRoot)
        throws IOException {
        if (file.isDirectory()) {
            // 处理文件夹
            String folderName;
            if (isRoot) {
                folderName = StringUtils.isEmpty(parentFolderName)
                    ? parentFolderName + File.separator + file.getName()
                    : file.getName();
            } else {
                folderName = StringUtils.isNotEmpty(parentFolderName) ? parentFolderName + File.separator
                    + file.getName() : file.getName();
            }
            zos.putNextEntry(new ZipEntry(folderName + File.separator));

            File[] files = file.listFiles();
            // 递归添加文件夹中的文件
            for (File nestedFile : files) {
                addFilesToZip(nestedFile, folderName, zos, false);
            }
        } else {
            // 处理文件
            byte[] buffer = new byte[LegoNumberConstant.BYTE_SIZE];
            try (FileInputStream fis = new FileInputStream(file)) {
                // 创建压缩包条目
                ZipEntry zipEntry;
                if (isRoot) {
                    zipEntry = new ZipEntry(parentFolderName);
                } else {
                    zipEntry = new ZipEntry(parentFolderName + File.separator + file.getName());
                }
                zos.putNextEntry(zipEntry);

                // 读取文件并写入压缩包
                int length;
                while ((length = fis.read(buffer)) > 0) {
                    zos.write(buffer, 0, length);
                }
            }
            zos.closeEntry();
        }
    }

    private static long writeToFile(ZipArchiveInputStream zis, ZipArchiveEntry entry, String unzipPath)
        throws IOException {
        String entryName = entry.getName();
        if (entryName.contains(FileUtil.DOUBLE_DOT_SLASH)) {
            logger.error("unzip file fail, file name unsafe, entry name : {}.", entry.getName());
            throw new LegoCheckedException("unzip file fail, file name unsafe.");
        }
        File file = new File(unzipPath, entryName);

        long entrySize = 0L;
        if (entry.isDirectory()) {
            // 创建文件夹
            file.mkdirs();
        } else {
            // 创建父文件夹
            File parent = file.getParentFile();
            if (!parent.exists()) {
                parent.mkdirs();
            }

            // 写入文件内容
            entrySize = writeContent(zis, entry, file);
        }
        return entrySize;
    }

    private static long writeContent(ZipArchiveInputStream zis, ZipArchiveEntry entry, File file) throws IOException {
        long entrySize = 0L;
        try (FileOutputStream fos = new FileOutputStream(file)) {
            byte[] buffer = new byte[LegoNumberConstant.BYTE_SIZE];
            int length = 0;
            while ((length = zis.read(buffer)) > 0) {
                fos.write(buffer, 0, length);

                entrySize += length;
                if (entrySize >= GB_BYTE_NUM * 2L) {
                    logger.error("unzip fail, zip entry is too big, name: {}, size: {}", entry.getName(), entrySize);
                    throw new LegoCheckedException("zip file entry size is too big.");
                }
            }
        }
        return entrySize;
    }

    /**
     * zip校验
     *
     * @param multipartFile 文件
     * @param fileCheckRule 校验参数
     * @return 校验结果
     */
    @Override
    public boolean check(MultipartFile multipartFile, FileCheckRule fileCheckRule) {
        // 炸弹攻击防护
        String uuid = IdUtil.simpleUUID();
        String zipPath = fileCheckRule.getTempPath() + File.separator + uuid;
        try (InputStream inputStream = multipartFile.getInputStream()) {
            FileUtils.copyInputStreamToFile(inputStream, new File(zipPath));
        } catch (IOException e) {
            return false;
        }

        try (FileInputStream fis = new FileInputStream(zipPath);
            ZipArchiveInputStream zis = new ZipArchiveInputStream(fis)) {
            if (!bombAttackCheck(fileCheckRule, zis)) {
                return false;
            }
        } catch (IOException | LegoCheckedException e) {
            logger.error("unzip error", ExceptionUtil.getErrorMessage(e));
            return false;
        } finally {
            FileUtil.deleteDir(new File(zipPath));
        }
        return true;
    }

    private static boolean bombAttackCheck(FileCheckRule fileCheckRule, ZipArchiveInputStream zis) throws IOException {
        ZipArchiveEntry entry;
        long totalSize = 0L;
        long entryNum = 0L;
        int entryDepth = 0;
        while ((entry = zis.getNextZipEntry()) != null) {
            // 文件夹不校验
            if (entry.isDirectory()) {
                entryDepth++;
                // 深度校验
                if (entryDepth > fileCheckRule.getMaxDepth()) {
                    logger.error("zip file entry depth is too many, num: {}", entryDepth);
                    return false;
                }
                continue;
            }

            // 深度清0
            entryDepth = 0;

            // 文件数量校验
            entryNum++;
            if (entryNum > fileCheckRule.getMaxEntryNum()) {
                logger.error("zip file entry is too many, num: {}", entryNum);
                return false;
            }

            // 单个文件大小，文件不写入磁盘，只校验大小
            totalSize += getEntrySize(zis, entry, fileCheckRule);
            if (totalSize >= fileCheckRule.getMaxUnZipSize()) {
                logger.error("unzip fail, total size is too big, name: {}, size: {}", entry.getName(), totalSize);
                return false;
            }
        }
        return true;
    }

    private static long getEntrySize(ZipArchiveInputStream zis, ZipArchiveEntry entry, FileCheckRule fileCheckRule)
        throws IOException {
        String entryName = entry.getName();
        // 非法文件名
        if (entryName.contains(FileUtil.DOUBLE_DOT_SLASH)) {
            logger.error("unzip file fail, file name unsafe, entry name : {}.", entry.getName());
            throw new LegoCheckedException("Unzip file fail, file name unsafe.");
        }
        entryName.chars().boxed().forEach(intChar -> {
            if (FileUtil.ILLEGAL_FILENAME_CHARS.contains(intChar)) {
                throw new LegoCheckedException("Unzip file fail, file name unsafe.");
            }
        });
        // 非法文件名
        if (entryName.contains(FileUtil.ILLEGAL_FILENAME_LINUX) || entryName.contains(
            FileUtil.ILLEGAL_FILENAME_WINDOWS)) {
            throw new LegoCheckedException("Unzip file fail, file name unsafe.");
        }

        // 内部文件白名单校验
        if (CollectionUtils.isNotEmpty(fileCheckRule.getZipWhiteList()) && !fileCheckRule.getZipWhiteList()
            .contains(entryName)) {
            throw new LegoCheckedException("File not in white list.");
        }

        long entrySize = 0L;
        // 获取entry大小
        byte[] buffer = new byte[LegoNumberConstant.BYTE_SIZE];
        int length;
        while ((length = zis.read(buffer)) > 0) {
            entrySize += length;
            if (entrySize >= fileCheckRule.getMaxEntrySize()) {
                logger.error("unzip fail, zip entry is too big, name: {}, size: {}", entry.getName(), entrySize);
                throw new LegoCheckedException("Zip file entry size is too big.");
            }
        }
        return entrySize;
    }
}
