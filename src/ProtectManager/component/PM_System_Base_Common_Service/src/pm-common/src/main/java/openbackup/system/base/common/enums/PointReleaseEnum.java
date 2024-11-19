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

import org.apache.commons.lang3.StringUtils;

/**
 * 版本
 *
 */
public enum PointReleaseEnum {
    /**
     * 1.2.1
     */
    V_1_2_1("1.2.1"),

    /**
     * 1.3.0
     */
    V_1_3_0("1.3.0"),

    /**
     * 1.5.0
     */
    V_1_5_0("1.5.0");

    private final String version;

    PointReleaseEnum(String version) {
        this.version = version;
    }

    public String getVersion() {
        return version;
    }

    /**
     * 是否是1.2.1版本
     *
     * @param version version
     * @return true-是，false-否
     */
    public static boolean isV121(String version) {
        return StringUtils.equals(version, V_1_2_1.getVersion());
    }

    /**
     * 是否是1.3.0版本
     *
     * @param version version
     * @return true-是，false-否
     */
    public static boolean isV130(String version) {
        return StringUtils.equals(version, V_1_3_0.getVersion());
    }
}