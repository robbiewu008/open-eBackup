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
package openbackup.database.base.plugin.enums;

/**
 * 集群实例在线状态策略
 *
 */
public enum ClusterInstanceOnlinePolicy {
    /**
     * 所有节点实例在线，集群实例在线
     */
    ALL_NODES_ONLINE("1"),

    /**
     * 任意节点实例在线，集群实例在线
     */
    ANY_NODE_ONLINE("2");

    private final String policy;

    /**
     * 构造方法
     *
     * @param policy 策略类型
     */
    ClusterInstanceOnlinePolicy(String policy) {
        this.policy = policy;
    }

    public String getPolicy() {
        return policy;
    }
}
