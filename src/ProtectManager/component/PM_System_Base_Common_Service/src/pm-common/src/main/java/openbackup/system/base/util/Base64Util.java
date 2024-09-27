/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

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
 * @author dWX1009286
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-16
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
}
