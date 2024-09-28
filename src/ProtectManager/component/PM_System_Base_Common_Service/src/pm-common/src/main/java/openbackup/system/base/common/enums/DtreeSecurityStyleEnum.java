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

/**
 * Dtree安全模式
 *
 */
public enum DtreeSecurityStyleEnum {
    NATIVE("1"),

    NTFS("2"),

    UNIX("3"),

    MIXED("4");

    private final String style;

    DtreeSecurityStyleEnum(String style) {
        this.style = style;
    }

    public String getStyle() {
        return style;
    }
}
