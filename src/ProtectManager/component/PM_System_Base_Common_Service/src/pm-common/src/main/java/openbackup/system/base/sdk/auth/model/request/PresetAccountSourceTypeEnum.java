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
package openbackup.system.base.sdk.auth.model.request;

/**
 * 来源枚举
 *
 * @author y30021475
 * @since 2023-08-07
 */
public enum PresetAccountSourceTypeEnum {
    /**
     * 来源平台 hcs
     */
    HCS("hcs");

    private final String sourceType;

    /**
     * 构造方法
     *
     * @param sourceType sourceType
     */
    PresetAccountSourceTypeEnum(String sourceType) {
        this.sourceType = sourceType;
    }

    public String getValue() {
        return sourceType;
    }
}
