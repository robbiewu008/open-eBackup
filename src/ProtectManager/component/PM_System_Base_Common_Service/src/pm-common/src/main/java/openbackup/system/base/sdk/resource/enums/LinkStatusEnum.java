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
package openbackup.system.base.sdk.resource.enums;

import lombok.Getter;

import java.util.Arrays;
import java.util.Objects;

/**
 * 受保护环境连接状态
 *
 * @author y00559272
 * @version [A8000 1.0.0]
 * @since 2021-03-08
 */
@Getter
public enum LinkStatusEnum {
    ONLINE(1),
    OFFLINE(0),
    // 升级排队中
    AGENT_STATUS_QUEUED(3),
    // 升级中
    AGENT_STATUS_UPDATING(4),
    // 不可用
    UNAVAILABLE(5),
    // 降级中
    DEGRADED(6),
    // 未启动
    UNSTARTED(7),
    // 适配集群，部分在线
    PARTLY_ONLING(8),
    // 实例异常
    ABNORMAL(9),
    /**
     * 部分连通
     */
    PARTIALLY_CONNECTED(10);

    private final Integer status;

    LinkStatusEnum(Integer status) {
        this.status = status;
    }

    /**
     * 根据状态值获取连接状态的枚举
     *
     * @param status 连接状态
     * @return LinkStatusEnum
     */
    public static LinkStatusEnum getByStatus(Integer status) {
        return Arrays.stream(LinkStatusEnum.values())
            .filter(linkStatus -> Objects.equals(linkStatus.status, status))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
