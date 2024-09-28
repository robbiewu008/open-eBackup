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
package openbackup.data.access.framework.protection.common.constants;

/**
 * 归档策略常量的key定义
 *
 **/
public class ArchivePolicyKeyConstant {
    /**
     * 归档策略扩展参数的key
     */
    public static final String EXT_PARAMETERS_KEY = "ext_parameters";

    /**
     * 归档策略扩展参数中存储库id的key
     */
    public static final String STORAGE_ID_KEY = "storage_id";

    /**
     * 归档策略扩展参数中存储库列表的key
     */
    public static final String STORAGE_LIST_KEY = "storage_list";

    /**
     * 归档策略扩展参数中存储库esn的key
     */
    public static final String ESN = "esn";

    /**
     * 归档策略扩展参数中存储库协议的key
     */
    public static final String PROTOCOL_KEY = "protocol";

    /**
     * 归档策略扩展参数中是否开启网络加速
     */
    public static final String NET_SPEED_UP_KEY = "network_access";

    /**
     * 归档策略扩展参数中是否开启自动索引
     */
    public static final String AUTO_INDEX_KEY = "auto_index";

    /**
     * 手动归档保留时间单位
     */
    public static final String DURATION_UNIT = "duration_unit";

    /**
     * 手动归档保留时间
     */
    public static final String RETENTION_DURATION = "retention_duration";

    /**
     * 手动归档保留类型
     */
    public static final String RETENTION_TYPE = "retention_type";

    /**
     * 归档目标类型：归档全部副本/归档指定副本
     */
    public static final String ARCHIVE_TARGET_TYPE = "archive_target_type";

    /**
     * 集群节点ip
     */
    public static final String IP = "ip";

    /**
     * 节点类型 0:主节点  1:从节点
     */
    public static final String CLUSTER_ROLE = "role";


    /**
     *  集群端口
     */
    public static final String CLUSTER_PORT = "port";

    /**
     *  备份存储id
     */
    public static final String BACKUP_STORAGE_ID = "backup_storage_id";
}
