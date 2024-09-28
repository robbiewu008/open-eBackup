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
import lombok.experimental.SuperBuilder;

import java.util.List;

/**
 * 压缩文件校验规则
 *
 */
@Data
@SuperBuilder
public class ZipFileValidateRule extends FileValidateRule {
    // 压缩文件中子文件最大允许数量
    private long fileMaxNum;

    // 压缩文件临时存放目录
    private String tempPath;

    // 压缩炸弹校验时，指定的解压文件临时存放路径
    private String unzipTempPath;

    // 解压文件后的文件大小最大允许值
    private long maxDecompressSize;

    // 解压文件包含的子文件列表
    private List<String> subFileList;
}
