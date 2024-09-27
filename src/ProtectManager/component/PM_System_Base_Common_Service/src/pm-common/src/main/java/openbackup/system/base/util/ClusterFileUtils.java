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

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.fileupload.disk.DiskFileItem;
import org.springframework.http.MediaType;
import org.springframework.web.multipart.MultipartFile;
import org.springframework.web.multipart.commons.CommonsMultipartFile;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;

/**
 * 文件处理的工具类
 *
 * @author y30046482
 * @since 2023-05-17
 */
@Slf4j
public class ClusterFileUtils {
    /**
     * 读取指定位置的文件，封装到MultipartFile对象里
     *
     * @param path 文件路径
     * @return MultipartFile 包含指定文件的
     */
    public static MultipartFile createMultipartFile(String path) {
        DiskFileItem item;
        try {
            File file = new File(path);
            item = new DiskFileItem("file", MediaType.MULTIPART_FORM_DATA_VALUE,
                true, file.getName(),
                (int) file.length(), file.getParentFile());
            OutputStream os = item.getOutputStream();
            os.write(org.apache.commons.io.FileUtils.readFileToByteArray(file));
        } catch (IOException e) {
            log.error("create multipart file failed.", ExceptionUtil.getErrorMessage(e));
            throw LegoCheckedException.cast(e);
        }
        return new CommonsMultipartFile(item);
    }

    /**
     * 删除指定位置的文件
     *
     * @param path 文件路径
     */
    public static void deleteFile(String path) {
        File file = new File(path);
        if (!file.exists()) {
            return;
        }
        file.delete();
    }
}