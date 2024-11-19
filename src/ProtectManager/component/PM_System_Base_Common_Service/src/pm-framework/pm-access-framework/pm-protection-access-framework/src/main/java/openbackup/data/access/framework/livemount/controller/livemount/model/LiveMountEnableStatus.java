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
package openbackup.data.access.framework.livemount.controller.livemount.model;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import openbackup.system.base.util.EnumUtil;

/**
 * Live Mount Enable Status
 *
 */
public enum LiveMountEnableStatus {
    /**
     * ACTIVATED
     */
    ACTIVATED("activated"),

    /**
     * DISABLED
     */
    DISABLED("disabled");

    private String name;

    LiveMountEnableStatus(String name) {
        this.name = name;
    }

    @JsonValue
    public String getName() {
        return name;
    }

    /**
     * get enum by name
     *
     * @param name name
     * @return enum
     */
    @JsonCreator
    public static LiveMountEnableStatus get(String name) {
        return EnumUtil.get(LiveMountEnableStatus.class, LiveMountEnableStatus::getName, name);
    }
}
