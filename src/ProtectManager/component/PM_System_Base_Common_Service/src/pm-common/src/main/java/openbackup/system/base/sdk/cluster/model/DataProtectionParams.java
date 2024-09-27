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
package openbackup.system.base.sdk.cluster.model;

import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Data protection params
 *
 * @author p30001902
 * @since 2020-12-17
 */
@Data
@NoArgsConstructor
public class DataProtectionParams {
    private String engineType;

    private String ipV4Address;

    private String ipV6Address;

    /**
     * 构造器
     *
     * @param engineType 网络平面类型
     * @param ip IPV4
     */
    public DataProtectionParams(String engineType, String ip) {
        this(engineType, ip, null);
    }

    /**
     * 构造器
     *
     * @param engineType 网络平面类型
     * @param ipv4 IPV4
     * @param ipv6 IPV6
     */
    public DataProtectionParams(String engineType, String ipv4, String ipv6) {
        this.engineType = engineType;
        this.ipV4Address = ipv4;
        this.ipV6Address = ipv6;
    }
}
