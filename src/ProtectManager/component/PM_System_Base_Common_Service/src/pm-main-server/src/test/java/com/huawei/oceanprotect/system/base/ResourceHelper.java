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
package com.huawei.oceanprotect.system.base;

import org.apache.commons.io.IOUtils;
import org.springframework.mock.web.MockMultipartFile;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

/**
 * 资源工具类
 *
 */
public class ResourceHelper {
    /**
     * excel文件的mime类型
     */
    public static final String EXCEL_MIME_TYPE = "application/vnd.ms-excel";

    /**
     * multipart名字
     */
    public static final String MULTIPART_FILE_NAME = "file";

    /**
     * 将参数文件资源内容读到string中
     *
     * @param clz clz
     * @param path path
     * @return string
     * @param <T> T
     */
    public static <T> String getResourceAsString(Class<T> clz, String path) {
        try (InputStream is = clz.getResourceAsStream(path)) {
            return IOUtils.toString(is, StandardCharsets.UTF_8);
        } catch (IOException e) {
            throw new RuntimeException(String.format("Get param files:%s error", path), e);
        }
    }

    /**
     * 通过参数文件创建multipartfile
     *
     * @param clz clz
     * @param name name
     * @return multipartfile
     * @param <T> T
     */
    public static <T> MultipartFile createMultipartFile(Class<T> clz, String name) {
        // 将字符串转换为byte数组
        byte[] fileContent;
        try (InputStream is = clz.getResourceAsStream(name)) {
            assert is != null;
            fileContent = IOUtils.toByteArray(is);
        } catch (IOException e) {
            throw new RuntimeException(String.format("Get param files:%s error", name), e);
        }

        // 使用MockMultipartFile创建MultipartFile实例
        return new MockMultipartFile(
            MULTIPART_FILE_NAME, // 这是表单中文件输入字段的名称
            name, // 这是上传文件的名称
            EXCEL_MIME_TYPE, // 这是文件的MIME类型
            fileContent // 这是文件的字节内容
        );
    }
}
