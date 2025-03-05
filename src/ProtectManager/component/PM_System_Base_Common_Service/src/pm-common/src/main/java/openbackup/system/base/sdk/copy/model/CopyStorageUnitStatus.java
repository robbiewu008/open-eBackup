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

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * Copy Storage Unit Status
 *
 */
public enum CopyStorageUnitStatus {
    /**
     * 存储单元在线
     */
    ONLINE(1),
    /**
     * 存储单元离线
     */
    OFFLINE(0);


    private final int value;

    CopyStorageUnitStatus(int value) {
        this.value = value;
    }

    @JsonValue
    public int getValue() {
        return value;
    }
}
