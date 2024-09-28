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
package openbackup.mongodb.protection.access.bo;

import lombok.Getter;
import lombok.Setter;

/**
 * MongoDB常量
 *
 */
@Getter
@Setter
public class MongoClusterNodesExtendInfo {
    // 备份路径
    private String dataPath;

    // 实例角色 1-主 2-备 7-仲裁
    private String role;

    // 投票权,默认空
    private String voteRight;

    // 优先级,默认空
    private String priority;

    // 备份配置文件路径
    private String configPath;

    // 实例 IP:Port
    private String hostUrl;

    // 下发参数的 IP:Port
    private String agentHost;

    // 实例状态1-在线 0-离线
    private String nodeStatus;

    /**
     * 节点类型
     */
    private String shardClusterType;

    /**
     * 节点角色
     */
    private String stateStr;

    /**
     * 节点启动参数
     */
    private String argv;

    /**
     * 备份配置文件所有信息
     */
    private String parsed;

    /**
     * 集群实例名称
     */
    private String clusterInstanceName;

    /**
     * 集群实例名称
     */
    private String shardInstanceName;

    /**
     * 分片集群
     */
    private String shardIndex;

    /**
     * 集群实例名称
     */
    private String instanceNameInfos;

    /**
     * 集群实例id,不唯一
     */
    private String instanceId;
}
