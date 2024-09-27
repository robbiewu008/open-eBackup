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
package openbackup.data.protection.access.provider.sdk.enums;

/**
 * 副本类型枚举类
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-11
 */
public enum CopyFormatEnum {
    /**
     * 快照格式:原生格式
     */
    INNER_SNAPSHOT(0),

    /**
     * 目录格式:非原生格式
     */
    INNER_DIRECTORY(1),

    /**
     * 外部格式
     */
    EXTERNAL(2);

    private final int copyFormat;

    CopyFormatEnum(int copyFormat) {
        this.copyFormat = copyFormat;
    }

    /**
     * getter
     *
     * @return copyFormat
     */
    public int getCopyFormat() {
        return copyFormat;
    }
}
