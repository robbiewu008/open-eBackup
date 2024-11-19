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
 * Live Mount Update Mode
 *
 */
public enum LiveMountUpdateMode {
    /**
     * LATEST
     */
    LATEST("latest"),
    /**
     * SPECIFIED
     */
    SPECIFIED("specified");

    private final String value;

    LiveMountUpdateMode(String value) {
        this.value = value;
    }

    /**
     * get enum by value
     *
     * @param value value
     * @return enum
     */
    @JsonCreator
    public static LiveMountUpdateMode get(String value) {
        return EnumUtil.get(LiveMountUpdateMode.class, LiveMountUpdateMode::getValue, value);
    }

    @JsonValue
    public String getValue() {
        return value;
    }
}
