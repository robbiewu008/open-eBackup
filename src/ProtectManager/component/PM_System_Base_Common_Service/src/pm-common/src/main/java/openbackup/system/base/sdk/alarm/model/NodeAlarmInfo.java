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
package openbackup.system.base.sdk.alarm.model;

import lombok.Data;

/**
 * 节点告警统计数据
 *
 */
@Data
public class NodeAlarmInfo {
    /**
     * 节点esn
     */
    private String esn;

    /**
     * 节点名称
     */
    private String nodeName;

    /**
     * 节点角色
     */
    private String nodeRole;

    /**
     * 关键告警
     */
    private int critical;

    /**
     * 警告告警
     */
    private int warning;

    /**
     * 主要告警
     */
    private int major;

    /**
     * 次要告警
     */
    private int minor;
}
