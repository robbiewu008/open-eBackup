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
package openbackup.system.base.common.enums;

import lombok.Getter;

/**
 * 同步文件到所有节点的文件类型枚举类
 *
 */
@Getter
public enum SyncFileTypeEnum {
    /**
     * 文本文件
     */
    TEXT("TEXT"),

    /**
     * 二进制文件
     */
    BINARY("BINARY");

    private final String type;

    SyncFileTypeEnum(String type) {
        this.type = type;
    }
}
