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
package openbackup.system.base.common.validator;

import openbackup.system.base.common.annotation.FileCheck;
import openbackup.system.base.common.enums.FileTypeEnum;
import openbackup.system.base.common.utils.files.FileUtil;
import openbackup.system.base.common.validator.FileValidator;

import org.apache.commons.collections.CollectionUtils;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.mock.web.MockMultipartFile;
import org.springframework.web.multipart.MultipartFile;

import java.util.Arrays;
import java.util.Collections;
import java.util.stream.Collectors;

import javax.validation.ConstraintValidatorContext;

/**
 * 功能描述
 *
 * @author w00607005
 * @since 2023-10-11
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {Collections.class, Arrays.class, FileUtil.class, Collectors.class, CollectionUtils.class})
@PowerMockIgnore( {"javax.management.*", "jdk.internal.reflect.*"})
public class FileValidatorTest {
    private FileValidator fileValidator;

    @Before
    public void setUp() {
        fileValidator = new FileValidator();
    }

    @Test
    public void test_initialize_success() throws Exception {
        // setup
        FileCheck fileCheck = Mockito.mock(FileCheck.class);
        PowerMockito.when(fileCheck.value()).thenReturn(FileTypeEnum.OTHERS);
        PowerMockito.when(fileCheck.maxSize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxNameLength()).thenReturn(0);
        PowerMockito.when(fileCheck.allowedFormats()).thenReturn(new String[]{});
        PowerMockito.when(fileCheck.tempPath()).thenReturn("string");
        PowerMockito.when(fileCheck.maxDepth()).thenReturn(0);
        PowerMockito.when(fileCheck.maxUnzipSize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntrySize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntryNum()).thenReturn(0);
        PowerMockito.when(fileCheck.zipWhiteList()).thenReturn(new String[]{});

        // run the test
        fileValidator.initialize(fileCheck);
    }

    /**
     * 用例场景：文件格式不符合要求
     * 前置条件：文件存在
     * 检查点：返回false
     */
    @Test
    public void test_is_valid_when_format_error() throws Exception {
        FileCheck fileCheck = Mockito.mock(FileCheck.class);
        PowerMockito.when(fileCheck.value()).thenReturn(FileTypeEnum.OTHERS);
        PowerMockito.when(fileCheck.maxSize()).thenReturn(1L);
        PowerMockito.when(fileCheck.maxNameLength()).thenReturn(0);
        PowerMockito.when(fileCheck.allowedFormats()).thenReturn(new String[]{"zip"});
        PowerMockito.when(fileCheck.tempPath()).thenReturn("string");
        PowerMockito.when(fileCheck.maxDepth()).thenReturn(0);
        PowerMockito.when(fileCheck.maxUnzipSize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntrySize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntryNum()).thenReturn(0);
        PowerMockito.when(fileCheck.zipWhiteList()).thenReturn(new String[]{});

        // run the test
        fileValidator.initialize(fileCheck);

        byte[] content = "File content".getBytes();
        MultipartFile multipartFile = new MockMultipartFile("file", "file.txt", "text/plain", content);

        ConstraintValidatorContext constraintValidatorContext = Mockito.mock(ConstraintValidatorContext.class);

        // run the test
        boolean result = fileValidator.isValid(multipartFile, constraintValidatorContext);

        // verify the results
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：文件名称长度不符合要求
     * 前置条件：文件存在
     * 检查点：返回false
     */
    @Test
    public void test_is_valid_when_file_name_length_error() throws Exception {
        FileCheck fileCheck = Mockito.mock(FileCheck.class);
        PowerMockito.when(fileCheck.value()).thenReturn(FileTypeEnum.OTHERS);
        PowerMockito.when(fileCheck.maxSize()).thenReturn(1L);
        PowerMockito.when(fileCheck.maxNameLength()).thenReturn(2);
        PowerMockito.when(fileCheck.allowedFormats()).thenReturn(new String[]{"txt"});
        PowerMockito.when(fileCheck.tempPath()).thenReturn("string");
        PowerMockito.when(fileCheck.maxDepth()).thenReturn(0);
        PowerMockito.when(fileCheck.maxUnzipSize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntrySize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntryNum()).thenReturn(0);
        PowerMockito.when(fileCheck.zipWhiteList()).thenReturn(new String[]{});

        // run the test
        fileValidator.initialize(fileCheck);

        byte[] content = "File content".getBytes();
        MultipartFile multipartFile = new MockMultipartFile("file", "file.txt", "text/plain", content);

        ConstraintValidatorContext constraintValidatorContext = Mockito.mock(ConstraintValidatorContext.class);

        // run the test
        boolean result = fileValidator.isValid(multipartFile, constraintValidatorContext);

        // verify the results
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：文件名称不符合要求
     * 前置条件：文件存在
     * 检查点：返回false
     */
    @Test
    public void test_is_valid_when_file_name_error() throws Exception {
        FileCheck fileCheck = Mockito.mock(FileCheck.class);
        PowerMockito.when(fileCheck.value()).thenReturn(FileTypeEnum.OTHERS);
        PowerMockito.when(fileCheck.maxSize()).thenReturn(1L);
        PowerMockito.when(fileCheck.maxNameLength()).thenReturn(100);
        PowerMockito.when(fileCheck.allowedFormats()).thenReturn(new String[]{"txt"});
        PowerMockito.when(fileCheck.tempPath()).thenReturn("string");
        PowerMockito.when(fileCheck.maxDepth()).thenReturn(0);
        PowerMockito.when(fileCheck.maxUnzipSize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntrySize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntryNum()).thenReturn(0);
        PowerMockito.when(fileCheck.zipWhiteList()).thenReturn(new String[]{});

        // run the test
        fileValidator.initialize(fileCheck);

        byte[] content = "File content".getBytes();
        MultipartFile multipartFile = new MockMultipartFile("file", "file../.txt", "text/plain", content);

        ConstraintValidatorContext constraintValidatorContext = Mockito.mock(ConstraintValidatorContext.class);

        // run the test
        boolean result = fileValidator.isValid(multipartFile, constraintValidatorContext);

        // verify the results
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：文件名称不符合要求
     * 前置条件：文件存在
     * 检查点：返回false
     */
    @Test
    public void test_is_valid_when_file_name_error1() throws Exception {
        FileCheck fileCheck = Mockito.mock(FileCheck.class);
        PowerMockito.when(fileCheck.value()).thenReturn(FileTypeEnum.OTHERS);
        PowerMockito.when(fileCheck.maxSize()).thenReturn(1L);
        PowerMockito.when(fileCheck.maxNameLength()).thenReturn(100);
        PowerMockito.when(fileCheck.allowedFormats()).thenReturn(new String[]{"txt"});
        PowerMockito.when(fileCheck.tempPath()).thenReturn("string");
        PowerMockito.when(fileCheck.maxDepth()).thenReturn(0);
        PowerMockito.when(fileCheck.maxUnzipSize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntrySize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntryNum()).thenReturn(0);
        PowerMockito.when(fileCheck.zipWhiteList()).thenReturn(new String[]{});

        // run the test
        fileValidator.initialize(fileCheck);

        byte[] content = "File content".getBytes();
        MultipartFile multipartFile = new MockMultipartFile("file", "file*.txt", "text/plain", content);

        ConstraintValidatorContext constraintValidatorContext = Mockito.mock(ConstraintValidatorContext.class);

        // run the test
        boolean result = fileValidator.isValid(multipartFile, constraintValidatorContext);

        // verify the results
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：文件名称不符合要求
     * 前置条件：文件存在
     * 检查点：返回false
     */
    @Test
    public void test_is_valid_when_file_name_error2() throws Exception {
        FileCheck fileCheck = Mockito.mock(FileCheck.class);
        PowerMockito.when(fileCheck.value()).thenReturn(FileTypeEnum.OTHERS);
        PowerMockito.when(fileCheck.maxSize()).thenReturn(1L);
        PowerMockito.when(fileCheck.maxNameLength()).thenReturn(100);
        PowerMockito.when(fileCheck.allowedFormats()).thenReturn(new String[]{"txt"});
        PowerMockito.when(fileCheck.tempPath()).thenReturn("string");
        PowerMockito.when(fileCheck.maxDepth()).thenReturn(0);
        PowerMockito.when(fileCheck.maxUnzipSize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntrySize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntryNum()).thenReturn(0);
        PowerMockito.when(fileCheck.zipWhiteList()).thenReturn(new String[]{});

        // run the test
        fileValidator.initialize(fileCheck);

        byte[] content = "File content".getBytes();
        MultipartFile multipartFile = new MockMultipartFile("file", "file./.txt", "text/plain", content);

        ConstraintValidatorContext constraintValidatorContext = Mockito.mock(ConstraintValidatorContext.class);

        // run the test
        boolean result = fileValidator.isValid(multipartFile, constraintValidatorContext);

        // verify the results
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：文件名称不符合要求
     * 前置条件：文件存在
     * 检查点：返回false
     */
    @Test
    public void test_is_valid_when_file_name_error3() throws Exception {
        FileCheck fileCheck = Mockito.mock(FileCheck.class);
        PowerMockito.when(fileCheck.value()).thenReturn(FileTypeEnum.OTHERS);
        PowerMockito.when(fileCheck.maxSize()).thenReturn(1L);
        PowerMockito.when(fileCheck.maxNameLength()).thenReturn(100);
        PowerMockito.when(fileCheck.allowedFormats()).thenReturn(new String[]{"txt"});
        PowerMockito.when(fileCheck.tempPath()).thenReturn("string");
        PowerMockito.when(fileCheck.maxDepth()).thenReturn(0);
        PowerMockito.when(fileCheck.maxUnzipSize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntrySize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntryNum()).thenReturn(0);
        PowerMockito.when(fileCheck.zipWhiteList()).thenReturn(new String[]{});

        // run the test
        fileValidator.initialize(fileCheck);

        byte[] content = "File content".getBytes();
        MultipartFile multipartFile = new MockMultipartFile("file", "file.\\.txt", "text/plain", content);

        ConstraintValidatorContext constraintValidatorContext = Mockito.mock(ConstraintValidatorContext.class);

        // run the test
        boolean result = fileValidator.isValid(multipartFile, constraintValidatorContext);

        // verify the results
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：文件大小不符合要求
     * 前置条件：文件存在
     * 检查点：返回false
     */
    @Test
    public void test_is_valid_when_file_size_error() throws Exception {
        FileCheck fileCheck = Mockito.mock(FileCheck.class);
        PowerMockito.when(fileCheck.value()).thenReturn(FileTypeEnum.OTHERS);
        PowerMockito.when(fileCheck.maxSize()).thenReturn(1L);
        PowerMockito.when(fileCheck.maxNameLength()).thenReturn(100);
        PowerMockito.when(fileCheck.allowedFormats()).thenReturn(new String[]{"txt"});
        PowerMockito.when(fileCheck.tempPath()).thenReturn("string");
        PowerMockito.when(fileCheck.maxDepth()).thenReturn(0);
        PowerMockito.when(fileCheck.maxUnzipSize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntrySize()).thenReturn(0L);
        PowerMockito.when(fileCheck.maxEntryNum()).thenReturn(0);
        PowerMockito.when(fileCheck.zipWhiteList()).thenReturn(new String[]{});

        // run the test
        fileValidator.initialize(fileCheck);

        byte[] content = "File content".getBytes();
        MultipartFile multipartFile = new MockMultipartFile("file", "file.txt", "text/plain", content);

        ConstraintValidatorContext constraintValidatorContext = Mockito.mock(ConstraintValidatorContext.class);

        // run the test
        boolean result = fileValidator.isValid(multipartFile, constraintValidatorContext);

        // verify the results
        Assert.assertFalse(result);
    }
}