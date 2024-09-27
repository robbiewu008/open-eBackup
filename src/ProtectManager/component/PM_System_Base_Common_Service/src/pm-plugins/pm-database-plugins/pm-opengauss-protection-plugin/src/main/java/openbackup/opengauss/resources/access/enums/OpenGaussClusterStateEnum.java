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
package openbackup.opengauss.resources.access.enums;

import com.google.common.collect.ImmutableSet;

import java.util.Set;
import java.util.stream.Collectors;

/**
 * OpenGauss集群状态枚举
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-13
 */
public enum OpenGaussClusterStateEnum {
    /**
     * 正常
     */
    NORMAL("Normal"),

    /**
     * 不可用
     */
    UNAVAILABLE("Unavailable"),

    /**
     * 降级
     */
    DEGRADED("Degraded"),

    /**
     * 异常
     */
    ABNORMAL("Abnormal");

    private final String state;

    /**
     * GaussDBTClusterStateEnum
     *
     * @param state 类型
     */
    OpenGaussClusterStateEnum(String state) {
        this.state = state;
    }

    /**
     * getter
     *
     * @return 类型
     */
    public String getState() {
        return state;
    }

    /**
     * NORMAL、DEGRADED代表集群可用
     *
     * @return state
     */
    public static Set<String> getOnlineClusterState() {
        return ImmutableSet.of(NORMAL, DEGRADED)
            .stream()
            .map(OpenGaussClusterStateEnum::getState)
            .collect(Collectors.toSet());
    }
}
