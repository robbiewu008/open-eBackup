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

import openbackup.system.base.common.utils.files.FileUtil;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.mock.web.MockMultipartFile;
import org.springframework.web.multipart.MultipartFile;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;

/**
 * File Util Test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({FileUtil.class, Files.class})
public class FileUtilTest {

    @Rule
    public final TemporaryFolder temporaryFolder = new TemporaryFolder();

    /**
     * 用例场景：更新文件的权限
     * 前置条件：1、文件存在
     * 检查点：不报错，正常结束
     */
    @Test
    public void update_file_permission_success_if_file_exist() throws IOException {
        File file = temporaryFolder.newFile("test.txt");
        PowerMockito.mockStatic(Files.class);
        PowerMockito.when(Files.setPosixFilePermissions(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);
        FileUtil.updateFilePermission(file, FileUtil.OWNER_640);
    }

    /**
     * 用例场景：更新文件的权限
     * 前置条件：1、文件不存在
     * 检查点：不报错，正常结束
     */
    @Test
    public void update_file_permission_success_if_file_not_exist() {
        File file = PowerMockito.mock(File.class);
        PowerMockito.when(file.exists()).thenReturn(false);
        FileUtil.updateFilePermission(file, FileUtil.OWNER_640);
    }

    /**
     * 用例场景：更新文件的权限
     * 前置条件：1、是文件夹
     * 检查点：不报错，正常结束
     */
    @Test
    public void update_file_permission_success_if_file_is_dir() {
        File file = PowerMockito.mock(File.class);
        PowerMockito.when(file.exists()).thenReturn(true);
        PowerMockito.when(file.isDirectory()).thenReturn(true);
        FileUtil.updateFilePermission(file, FileUtil.OWNER_640);
    }

    /**
     * 用例场景：获取文件格式
     * 前置条件：文件
     * 检查点：不报错，正常结束
     */
    @Test
    public void test_get_file_format() {
        byte[] content = "File content".getBytes();
        MultipartFile multipartFile = new MockMultipartFile("file", "file.txt", "text/plain", content);
        String format = FileUtil.getFileFormat(multipartFile);
        Assert.assertEquals("txt", format);
    }


    /**
     * 用例场景：获取文件名
     * 前置条件：文件
     * 检查点：不报错，正常结束
     */
    @Test
    public void test_get_file_name() {
        byte[] content = "File content".getBytes();
        MultipartFile multipartFile = new MockMultipartFile("file", "file.txt", "text/plain", content);
        String name = FileUtil.getFileName(multipartFile);
        Assert.assertEquals("file.txt", name);
    }


    /**
     * 用例场景：获取文件大小
     * 前置条件：文件
     * 检查点：不报错，正常结束
     */
    @Test
    public void test_get_file_size() {
        byte[] content = "File content".getBytes();
        MultipartFile multipartFile = new MockMultipartFile("file", "file.txt", "text/plain", content);
        long size = FileUtil.getFileSize(multipartFile);
        Assert.assertEquals(12, size);
    }
}
