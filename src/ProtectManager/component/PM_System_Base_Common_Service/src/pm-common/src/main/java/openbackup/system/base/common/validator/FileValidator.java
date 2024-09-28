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

import openbackup.system.base.bean.FileCheckRule;
import openbackup.system.base.common.annotation.FileCheck;
import openbackup.system.base.common.enums.FileTypeEnum;
import openbackup.system.base.common.utils.files.FileUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.springframework.web.multipart.MultipartFile;

import java.util.Arrays;
import java.util.Collections;
import java.util.Locale;
import java.util.Set;
import java.util.stream.Collectors;

import javax.validation.ConstraintValidator;
import javax.validation.ConstraintValidatorContext;

/**
 * 文件校验
 *
 */
@Slf4j
public class FileValidator implements ConstraintValidator<FileCheck, MultipartFile> {
    private FileTypeEnum fileTypeEnum;

    private final FileCheckRule checkRule = new FileCheckRule();

    /**
     * 初始化
     *
     * @param fileCheck 注解
     */
    @Override
    public void initialize(FileCheck fileCheck) {
        fileTypeEnum = fileCheck.value();
        checkRule.setMaxSize(fileCheck.maxSize());
        checkRule.setMaxNameLength(fileCheck.maxNameLength());
        checkRule.setAllowedFormats(Arrays.stream(fileCheck.allowedFormats()).collect(Collectors.toSet()));
        checkRule.setTempPath(fileCheck.tempPath());
        checkRule.setMaxDepth(fileCheck.maxDepth());
        checkRule.setMaxUnZipSize(fileCheck.maxUnzipSize());
        checkRule.setMaxEntrySize(fileCheck.maxEntrySize());
        checkRule.setMaxEntryNum(fileCheck.maxEntryNum());
        checkRule.setZipWhiteList(Arrays.stream(fileCheck.zipWhiteList()).collect(Collectors.toSet()));
        // 目前压缩包文件仅支持zip格式
        if (FileTypeEnum.ZIP.equals(fileTypeEnum) && fileCheck.allowedFormats().length == 0) {
            checkRule.setAllowedFormats(Collections.singleton("zip"));
        }
    }

    /**
     * 检验
     *
     * @param multipartFile 文件
     * @param constraintValidatorContext 上下文
     * @return 校验结果
     */
    @Override
    public boolean isValid(MultipartFile multipartFile, ConstraintValidatorContext constraintValidatorContext) {
        return baseCheck(multipartFile, checkRule) && fileTypeEnum.getFunction().check(multipartFile, checkRule);
    }

    private static boolean baseCheck(MultipartFile multipartFile, FileCheckRule checkRule) {
        // 文件格式白名单校验
        String suffix = FileUtil.getFileFormat(multipartFile);
        if (CollectionUtils.isNotEmpty(checkRule.getAllowedFormats()) && !checkRule.getAllowedFormats()
            .contains(suffix.toLowerCase(Locale.ROOT))) {
            log.error("invalid file type, type: {}", suffix);
            return false;
        }

        // 文件名校验
        String fileName = FileUtil.getFileName(multipartFile);
        if (fileName.length() > checkRule.getMaxNameLength()) {
            log.error("file name too long, name: {}", fileName);
            return false;
        }
        if (fileName.contains(FileUtil.DOUBLE_DOT_SLASH)) {
            log.error("invalid file name, name: {}", fileName);
            return false;
        }

        Set<Integer> fileNameCharSet = fileName.chars()
            .boxed()
            .filter(FileUtil.ILLEGAL_FILENAME_CHARS::contains)
            .collect(Collectors.toSet());
        if (CollectionUtils.isNotEmpty(fileNameCharSet)) {
            log.error("invalid file name, name: {}", fileName);
            return false;
        }

        if (fileName.contains(FileUtil.ILLEGAL_FILENAME_LINUX) || fileName.contains(
            FileUtil.ILLEGAL_FILENAME_WINDOWS)) {
            log.error("invalid file name, name: {}", fileName);
            return false;
        }

        // 文件大小校验
        long fileSize = FileUtil.getFileSize(multipartFile);
        if (fileSize > checkRule.getMaxSize()) {
            log.error("the file size exceeds the limit, actual size: {}", fileSize);
            return false;
        }
        return true;
    }
}
