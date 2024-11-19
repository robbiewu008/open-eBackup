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
 * The AlarmLang
 *
 */
public enum AlarmLang {
    EN(1, "en"),
    ZH(2, "zh");

    private final int value;
    private final String key;

    AlarmLang(final int value, final String key) {
        this.value = value;
        this.key = key;
    }

    /**
     * 获取语言value
     * 用于DM查询告警对象场景
     *
     * @return 语言信息
     */
    public int getValue() {
        return value;
    }

    /**
     * 获取语言Key
     * 用于DM查询告警场景
     *
     * @return 语言信息
     */
    public String getKey() {
        return key;
    }
}