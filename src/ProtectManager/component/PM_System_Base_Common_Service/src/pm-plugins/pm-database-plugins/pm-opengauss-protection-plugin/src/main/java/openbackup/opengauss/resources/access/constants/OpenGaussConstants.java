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
package openbackup.opengauss.resources.access.constants;

/**
 * OpenGauss相关常量
 *
 */
public class OpenGaussConstants {
    /**
     * 集群状态 Normal 为正常状态
     */
    public static final String CLUSTER_STATE = "clusterState";

    /**
     * 集群信息 Normal 为正常状态
     */
    public static final String CLUSTER_INFO = "clusterInfo";

    /**
     * 集群nodes信息
     */
    public static final String NODES = "nodes";

    /**
     * 集群角色对应的role
     */
    public static final String CLUSTER_NODE_ROLE = "role";

    /**
     * 前端下发的节点信息
     */
    public static final String GUI_NODES = "guiNodes";

    /**
     * 状态信息
     */
    public static final String STATUS = "status";

    /**
     * 分隔符息
     */
    public static final String PATH_DELIMITER = "/";

    /**
     * 数据库环境的唯一系统id
     */
    public static final String SYSTEM_ID = "systemId";

    /**
     * 单节点
     */
    public static final int SINGLE_NODE = 1;

    /**
     * OpenGauss注册集群上限值
     */
    public static final int OPENGAUSS_CLUSTER_MAX_COUNT = 2000;

    /**
     * 资源扫描时间间隔
     */
    public static final int OPENGAUSS_SCAN_INTERVAL = 300;

    /**
     * 分布式
     */
    public static final int DISTRIBUTED = 4;

    /**
     * 是否删除关联副本
     */
    public static final String IS_DELETE_RELATIVE_COPIES = "is_delete_relative_copies";

    /**
     * 实例状态
     */
    public static final String INSTANCE_STATE = "instanceState";
}
