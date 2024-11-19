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

import com.google.common.collect.Sets;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.bean.FileCheckRule;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;

import org.apache.commons.lang3.StringUtils;
import org.springframework.web.multipart.MultipartFile;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.CopyOption;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.stream.Collectors;

/**
 * File Util
 *
 */
@Slf4j
public class FileUtil implements FileCheckInterface {
    /**
     * 640 权限
     */
    public static final Set<PosixFilePermission> OWNER_640 = Sets.immutableEnumSet(PosixFilePermission.OWNER_READ,
            PosixFilePermission.OWNER_WRITE, PosixFilePermission.GROUP_READ);

    /**
     * 非法文件名常量
     */
    public static final String DOUBLE_DOT_SLASH = "../";

    /**
     * 非法的文件名字符，包括":*?\"<>|"
     */
    public static final Set<Integer> ILLEGAL_FILENAME_CHARS = ":*?\"<>|".chars().boxed().collect(Collectors.toSet());

    /**
     * linux环境的非法文件名格式常量
     */
    public static final String ILLEGAL_FILENAME_LINUX = "./";

    /**
     * windows环境的非法文件名格式常量
     */
    public static final String ILLEGAL_FILENAME_WINDOWS = ".\\";

    @Override
    public boolean check(MultipartFile multipartFile, FileCheckRule validateRule) {
        return true;
    }

    /**
     * 创建文件
     *
     * @param file file
     * @throws IOException io异常
     */
    public static void createFile(File file) throws IOException {
        if (!file.exists()) {
            file.createNewFile();
        }
    }

    /**
     * 创建目录
     *
     * @param file file
     */
    public static void mkdirs(File file) {
        mkdirs(file, file.getName());
    }

    /**
     * 创建目录
     *
     * @param file file
     * @param name name
     */
    public static void mkdirs(File file, String name) {
        if (!file.mkdirs()) {
            log.trace("file: {} may already exist", name);
        }
    }

    /**
     * clean
     *
     * @param file file
     * @return boolean
     */
    public static boolean clean(File file) {
        if (!file.exists()) {
            return true;
        }
        if (!file.isDirectory() && !file.delete()) {
            log.trace("file: {} may delete failed. exists: {}", file.getName(), file.exists());
            return file.exists();
        }
        File[] items = file.listFiles();
        if (items != null) {
            for (File item : items) {
                if (!clean(item)) {
                    return false;
                }
            }
        }

        return true;
    }

    /**
     * write text to file
     *
     * @param text text
     * @param file file
     * @throws IOException IOException
     */
    public static void write(String text, File file) throws IOException {
        try (FileOutputStream outputStream = new FileOutputStream(file)) {
            outputStream.write(text.getBytes(StandardCharsets.UTF_8));
            outputStream.flush();
        } catch (FileNotFoundException e) {
            throw new IOException("file missing: " + file.getName());
        }
        write(() -> new ByteArrayInputStream(text.getBytes(StandardCharsets.UTF_8)), file);
    }

    /**
     * write content to file
     *
     * @param supplier content supplier
     * @param file target file
     * @throws IOException IOException
     */
    public static void write(Callable<InputStream> supplier, File file) throws IOException {
        byte[] bytes = new byte[IsmNumberConstant.THROUND_TWENTY_FOUR];
        try (InputStream inputStream = supplier.call(); FileOutputStream outputStream = new FileOutputStream(file)) {
            while (true) {
                int length = -1;
                if (inputStream != null) {
                    length = inputStream.read(bytes);
                }
                if (length > 0) {
                    outputStream.write(bytes, 0, length);
                }
                if (length == -1) {
                    break;
                }
            }
        } catch (FileNotFoundException e) {
            throw new IOException("fail to open file to write. file: " + file.getName());
        } catch (IOException e) {
            throw e;
        } catch (Exception e) {
            throw new IOException(e);
        }
    }

    /**
     * 更新文件的权限为目标权限
     *
     * @param file 文件夹
     * @param permissions 目标权限
     */
    public static void updateFilePermission(File file, Set<PosixFilePermission> permissions) {
        if (!file.exists()) {
            log.warn("File not exist. so no update permission");
            return;
        }
        if (file.isDirectory()) {
            log.warn("File is dir. so no update permission");
            return;
        }
        Path path = file.toPath();
        try {
            Files.setPosixFilePermissions(path, permissions);
        } catch (IOException ioException) {
            log.error("Change permission error.", ExceptionUtil.getErrorMessage(ioException));
        }
    }

    /**
     * 递归删除文件
     *
     * @param dir 文件
     */
    public static void deleteDir(File dir) {
        if (dir.isDirectory()) {
            String[] children = dir.list();
            for (String child : children) {
                deleteDir(new File(dir, child));
            }
        }
        dir.delete();
    }

    /**
     * 递归删除文件
     *
     * @param path 文件路径
     */
    public static void deleteDir(String path) {
        deleteDir(new File(path));
    }

    /**
     * 读取文件内容到字节数组
     *
     * @param filePath 文件路径
     * @return 字节数组
     * @throws IOException IOException
     */
    public static byte[] readFileToByte(String filePath) throws IOException {
        Path path = Paths.get(filePath);
        return Files.readAllBytes(path);
    }

    /**
     * 读取文件内容到字符串
     *
     * @param filePath 文件路径
     * @return 字符串
     * @throws IOException IOException
     */
    public static String readFileToString(String filePath) throws IOException {
        return new String(readFileToByte(filePath), StandardCharsets.UTF_8);
    }

    /**
     * 将字节数组写入文件
     *
     * @param bytes 字节数组
     * @param filePath 文件路径
     * @throws IOException IOException
     */
    public static void writeToFile(byte[] bytes, String filePath) throws IOException {
        try (FileOutputStream fos = new FileOutputStream(filePath)) {
            fos.write(bytes);
        }
    }

    /**
     * 将字符串写入文件
     *
     * @param content 字节数组
     * @param filePath 文件路径
     * @throws IOException IOException
     */
    public static void writeToFile(String content, String filePath) throws IOException {
        byte[] bytes = content.getBytes(StandardCharsets.UTF_8);
        writeToFile(bytes, filePath);
    }

    /**
     * 获取文件格式
     *
     * @param file 文件
     * @return 文件类型
     */
    public static String getFileFormat(MultipartFile file) {
        String fileName = file.getOriginalFilename();
        if (StringUtils.isEmpty(fileName)) {
            log.error("file name does not exist. file: {}", fileName);
            return StringUtils.EMPTY;
        }
        String suffix = fileName.substring(fileName.lastIndexOf(".") + 1);
        if (StringUtils.isEmpty(suffix)) {
            log.error("file extension is empty. file: {}", fileName);
            return StringUtils.EMPTY;
        }
        return suffix;
    }

    /**
     * 获取文件名
     *
     * @param file 文件
     * @return 文件名
     */
    public static String getFileName(MultipartFile file) {
        return file.getOriginalFilename();
    }

    /**
     * 获取文件大小（单位：byte）
     *
     * @param file 文件
     * @return 文件大小
     */
    public static long getFileSize(MultipartFile file) {
        return file.getSize();
    }

    /**
     * 文件复制，不传options时，文件存在直接返回
     *
     * @param source source
     * @param target target
     * @param options options
     * @throws IOException IOException
     */
    public static void copy(Path source, Path target, CopyOption... options) throws IOException {
        if (VerifyUtil.isEmpty(options) && target.toFile().exists()) {
            return;
        }
        Files.copy(source, target, options);
    }
}
