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
package openbackup.system.base.controller;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.condition.ConditionalOnDeployType;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.util.Base64Util;

import org.apache.commons.io.FileUtils;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.multipart.MultipartFile;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

/**
 * 节点间的文件操作：同步，删除
 *
 */
@Slf4j
@RestController
@RequestMapping("/v1/internal/service")
@ConditionalOnDeployType(value = {DeployTypeEnum.E1000})
public class NodeFileController {
    /**
     * 同步文件到集群中的所有节点
     *
     * @param multipartFile dumpFile
     * @param filePath filePath
     */
    @ExterAttack
    @PostMapping("/syncfile")
    public void syncFile(@RequestParam("file") MultipartFile multipartFile,
        @RequestParam("filePath") String filePath) {
        log.info("Start sync file.");
        String decodeFilePath = Base64Util.decryptBase64ToString(filePath);
        File file = new File(decodeFilePath);
        if (!file.exists()) {
            try {
                FileUtils.copyInputStreamToFile(multipartFile.getInputStream(), file);
            } catch (IOException e) {
                log.error("Sync file failed.", ExceptionUtil.getErrorMessage(e));
            }
        } else {
            log.warn("file already exist, no need sync.");
        }
    }

    /**
     * 同步文件到集群中的所有节点
     *
     * @param filePath filePath
     */
    @ExterAttack
    @DeleteMapping("/deletefile")
    public void deleteFile(@RequestParam("filePath") String filePath) {
        String decodeFilePath = Base64Util.decryptBase64ToString(filePath);
        Path path = Paths.get(decodeFilePath);
        try {
            Files.deleteIfExists(path);
        } catch (IOException e) {
            log.error("fail delete file. filePath: {}", filePath);
        }
    }
}
