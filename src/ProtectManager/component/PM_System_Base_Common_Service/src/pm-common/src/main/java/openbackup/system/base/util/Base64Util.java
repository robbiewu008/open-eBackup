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

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang3.StringUtils;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Base64;
import java.util.Optional;

/**
 * Base64文本编码工具
 *
 */
@Slf4j
public class Base64Util {
    /**
     * 将路径为path的文件内容转为Base64码
     *
     * @param path 文件路径
     * @return 文件内容转码为Base64后的字符串
     */
    public static String encryptToBase64(Path path) {
        try {
            byte[] fileBytes = Files.readAllBytes(path);
            return Base64.getEncoder().encodeToString(fileBytes);
        } catch (IOException e) {
            log.error("Failed to encode file, path:{}, error message: {}", path, e.getMessage());
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED);
        }
    }

    /**
     * 将context文本内容转为Base64码
     *
     * @param context 文本
     * @return 文本转码为Base64后的字符串
     */
    public static String encryptToBase64(String context) {
        return Base64.getEncoder().encodeToString(context.getBytes(StandardCharsets.UTF_8));
    }

    /**
     * 将bytes内容转为Base64码
     *
     * @param bytes bytes
     * @return bytes转码为Base64后的字符串
     */
    public static String encryptToBase64(byte[] bytes) {
        return Base64.getEncoder().encodeToString(bytes);
    }

    /**
     * 将Base64编码的文本解码为原文本。
     *
     * @param context 已编码文本
     * @return 解码后的原文本
     */
    public static String decryptBase64ToString(String context) {
        String fileContext = Optional.ofNullable(context).orElse(StringUtils.EMPTY);
        return IOUtils.toString(Base64.getDecoder().decode(fileContext));
    }

    /**
     * 将Base64编码的文本解码为原文本。
     *
     * @param context 已编码文本
     * @return 解码后的原文本
     */
    public static byte[] decryptBase64ToBytes(String context) {
        String fileContext = Optional.ofNullable(context).orElse(StringUtils.EMPTY);
        return Base64.getDecoder().decode(fileContext);
    }
}
