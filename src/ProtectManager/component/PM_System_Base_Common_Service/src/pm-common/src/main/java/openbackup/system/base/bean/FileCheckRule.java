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
package openbackup.system.base.bean;

import lombok.Data;

import java.util.Set;

/**
 * 文件校验规则
 *
 */
@Data
public class FileCheckRule {
    /**
     * 文件名要求长度
     */
    private int maxNameLength;

    /**
     * 文件大小
     */
    private long maxSize;

    /**
     * 允许后缀
     */
    private Set<String> allowedFormats;

    /**
     * 临时目录
     */
    private String tempPath;

    /**
     * 最大深度
     */
    private int maxDepth;

    /**
     * 解压缩后最大大小
     */
    private long maxUnZipSize;

    /**
     * 单个entrySize大小
     */
    private long maxEntrySize;

    /**
     * 压缩文件内文件数量
     */
    private int maxEntryNum;

    /**
     * 压缩包内文件白名单
     */
    private Set<String> zipWhiteList;
}
