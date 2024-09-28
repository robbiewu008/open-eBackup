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

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.codec.digest.DigestUtils;
import org.apache.commons.io.FileUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * 校验文件
 *
 */
@Slf4j
public class FileCheckUtils {
    /**
     * 校验文件数量以及每个文件的签名是否一致
     *
     * @param filePath 文件路径
     * @param signFilePath 不参与校验的文件路径
     * @param listFileName 文件清单（记录了所有文件及其签名的文件）
     * @throws IOException io异常
     */
    public static void checkFileNumsAndSign(String filePath, Set<String> signFilePath, String listFileName)
        throws IOException {
        File rootPath = new File(filePath);
        List<File> files = (List<File>) FileUtils.listFiles(rootPath, null, true);
        List<File> fileList = files.stream().filter(file -> {
            try {
                String canonicalPath = file.getCanonicalFile().getPath();
                if (signFilePath.contains(canonicalPath)) {
                    return false;
                }
            } catch (IOException e) {
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "filter fileList error", e);
            }
            return true;
        }).collect(Collectors.toList());
        File syncFile = new File(filePath + File.separator + listFileName);
        List<String> syncFiles = FileUtils.readLines(syncFile, Charset.forName("utf-8"));
        // 比较文件数量是否一致
        if (syncFiles.size() != fileList.size()) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "verify file nums fail");
        }
        // 文件路径，签名
        Map<String, String> pathSignMap = new HashMap<>();
        for (String signAndPathStr : syncFiles) {
            String[] signAndPathArr = signAndPathStr.split("  ");
            if (signAndPathArr.length != 2) {
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "file content is not correct.");
            }
            String path = signAndPathArr[1].trim();
            path = FileUtils.getFile(filePath, path).getCanonicalFile().getPath();
            pathSignMap.put(path, signAndPathArr[0].trim());
        }
        // 比较现有的和sha256sum_sync文件中记录的
        for (File file : fileList) {
            // 看当前文件是否在sha256sum_sync中存在
            String path = file.getCanonicalFile().getPath();
            if (!pathSignMap.containsKey(path)) {
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "verify file sha256 fail");
            }
            String sha256 = genFileSHA256(file);
            if (!sha256.equals(pathSignMap.get(path))) {
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "verify file sha256 fail");
            }
        }
    }

    private static String genFileSHA256(File file) {
        try(InputStream is = new FileInputStream(file)) {
            return DigestUtils.sha256Hex(is);
        } catch (IOException e) {
            log.error("genFileSHA256 exception", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "genFileSHA256 exception");
        }
    }
}
