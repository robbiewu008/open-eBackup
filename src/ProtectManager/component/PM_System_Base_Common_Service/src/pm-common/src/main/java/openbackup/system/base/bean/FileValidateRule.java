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

/**
 * 通用文件校验规则
 *
 * @author hwx1144169
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-03-31
 */
@Data
@SuperBuilder
public class FileValidateRule {
    // 文件名要求长度
    private int nameMaxLength;

    // 文件后缀
    private String suffix;

    // 文件大小
    private long maxSize;

    // 文件路径
    private String path;

    // 文件路径规则正则表达式
    private String pathRule;
}
