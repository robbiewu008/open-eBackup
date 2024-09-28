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
package openbackup.system.base.sdk.copy.model;

import openbackup.system.base.util.EnumUtil;

import com.alibaba.fastjson.annotation.JSONCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * Copy Status
 *
 */
public enum CopyStatus {
    /**
     * INVALID
     */
    INVALID("Invalid"),
    /**
     * NORMAL
     */
    NORMAL("Normal"),
    /**
     * RESTORING
     */
    RESTORING("Restoring"),
    /**
     * MOUNTING
     */
    MOUNTING("Mounting"),

    /**
     * MOUNTED
     */
    MOUNTED("Mounted"),

    /**
     * UNMOUNTING
     */
    UNMOUNTING("Unmounting"),
    /**
     * DELETING
     */
    DELETING("Deleting"),
    /**
     * VERIFYING
     */
    VERIFYING("Verifying"),
    /**
     * Delete failed
     * */
    DELETEFAILED("DeleteFailed"),
    /**
     * Sharing
     * */
    SHARING("Sharing"),

    /**
     * Downloading
     */
    DOWNLOADING("Downloading");

    private final String value;

    CopyStatus(String value) {
        this.value = value;
    }

    /**
     * get enum by value
     *
     * @param value value
     * @return copy status
     */
    @JSONCreator
    public static CopyStatus get(String value) {
        return EnumUtil.get(CopyStatus.class, CopyStatus::getValue, value);
    }

    @JsonValue
    public String getValue() {
        return value;
    }
}
