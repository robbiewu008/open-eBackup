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
package openbackup.gaussdbdws.protection.access.constant;

import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import com.google.common.collect.ImmutableMap;

import java.util.Map;

/**
 * 功能描述: DWS模块的通用常量
 *
 */
public class DwsConstant {
    /**
     * 允许接入的DWS集群规格上限
     */
    public static final int GAUSSDB_DWS_CLUSTER_MAX_COUNT = 8;

    /**
     * 允许接入的DWS集群规格上限
     */
    public static final String DWS_CLUSTER_AGENT = "clusterAgent";

    /**
     * 允许接入的DWS集群规格上限
     */
    public static final String HOST_AGENT = "hostAgent";

    /**
     * coordinator_ip列表
     */
    public static final String COORDINATOR_IP = "coordinator_ip";

    /**
     * python 传递的key BACKUP_METADATA_PATH
     */
    public static final String ADVANCE_PARAMS_KEY_BACKUP_METADATA_PATH = "backup_metadata_path";

    /**
     * python 传递的key storage_id
     */
    public static final String ADVANCE_PARAMS_KEY_STORAGE_ID = "storage_id";


    /**
     * 传递给Agent的key METADATA_PATH
     */
    public static final String ADVANCE_PARAMS_KEY_METADATA_PATH = "metadataPath";

    /**
     * python 传递的key BACKUP_TOOL_TYPE
     */
    public static final String ADVANCE_PARAMS_KEY_BACKUP_TOOL_TYPE = "backup_tool_type";

    /**
     * 传递给Agent的key TOOL_TYPE
     */
    public static final String ADVANCE_PARAMS_KEY_TOOL_TYPE = "backupToolType";

    /**
     * 速率统计方式
     */
    public static final String SPEED_STATISTICS = "speedStatistics";

    /**
     * dws集群的版本信息 扩展信息 extendInfo 中 version 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_VERSION = "cluster_version";

    /**
     * dws集群的使用状态 扩展信息 extendInfo 中 version 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_STATE = "cluster_state";

    /**
     * dws集群的使用状态 扩展信息 extendInfo 中 resource_type 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_RESOURCE_TYPE = "resource_type";

    /**
     * dws 集群查询状态 正常
     */
    public static final String EXTEND_INFO_NORMAL_VALUE_STATE = "Normal";

    /**
     * dws 集群查询状态 不可用
     */
    public static final String EXTEND_INFO_UNAVAILABLE_VALUE_STATE = "Unavailable";

    /**
     * dws 集群查询状态 降级
     */
    public static final String EXTEND_INFO_DEGRADED_VALUE_STATE = "Degraded";

    /**
     * dws 集群查询状态 未启动
     */
    public static final String EXTEND_INFO_UNSTARTED_VALUE_STATE = "Unstarted";

    /**
     * 数据库 在扩展信息中的extendInfo key 值 database
     */
    public static final String EXTEND_INFO_KEY_DATABASES = "databases";

    /**
     * 创建备份集时，框架的 resource 扩展信息 extendInfo 中 DWS 的 schema/table 的 key 名称
     */
    public static final String EXTEND_INFO_KEY_TABLE = "table";

    /**
     * extendInfo 中 DWS 的 database 的 key 名称
     */
    public static final String EXTEND_INFO_KEY_DATABASE = "database";

    /**
     * 数据库 在扩展信息中的extendInfo key 值 ENV_FILE
     */
    public static final String EXTEND_INFO_KEY_ENV_FILE = "envFile";

    /**
     * 数据库 在扩展信息中的extendInfo key 值 EDWS_USER
     */
    public static final String EXTEND_INFO_KEY_DWS_USER = "DwsUser";

    /**
     * 数据库 在扩展信息中的extendInfo key 值 NODE_TYPE
     */
    public static final String EXTEND_INFO_KEY_NODE_TYPE = "nodeType";

    /**
     * 数据库 在扩展信息中的extendInfo value 值 CLUSTER_TYPE
     */
    public static final String EXTEND_INFO_VALUE_CLUSTER_TYPE = "0";

    /**
     * 数据库 在扩展信息中的extendInfo value 值 HOST_TYPE
     */
    public static final String EXTEND_INFO_VALUE_HOST_TYPE = "1";

    /**
     * 主机节点不能添加的接入版本
     */
    public static final String HOST_NO_ADD_VERSION = "8.0.0";

    /**
     * 多文件系统key名称
     */
    public static final String REPOSITORIES_KEY_MULTI_FILE_SYSTEM = "multiFileSystem";

    /**
     * ens key 名称
     */
    public static final String REPOSITORIES_KEY_ENS = "esn";

    /**
     * 恢复位置 key 名称
     */
    public static final String EXTEND_INFO_KEY_TARGET_LOCATION = "targetLocation";

    /**
     * 待恢复的子类型
     */
    public static final String EXTEND_INFO_KEY_SUB_TYPE = "resourceSubType";

    /**
     * CN节点key
     */
    public static final String CN_NODES = "cn_nodes";

    /**
     * CN节点key
     */
    public static final String TERMINAL_NODE = "terminal_node";

    /**
     * CN节点key
     */
    public static final String IAM_USER_ACCOUNT = "iam_user_account";

    /**
     * 项目的uuid
     */
    public static final String PROJECT_ID = "project_id";

    /**
     * 是否为项目扫描的
     */
    public static final String IS_PROJECT_SCAN = "isProjectScan";

    /**
     * 是否可删除
     */
    public static final String IS_DELETE = "isDelete";

    /**
     * 是否为项目
     */
    public static final String IS_PROJECT = "isProject";


    /**
     *  repository role 角色 master
     */
    public static final int MASTER_ROLE = 0;

    /**
     * repository role 角色 slave
     */
    public static final int SLAVE_ROLE = 1;

    /**
     * dws schema集的最大规格
     */
    public static final int SCHEMA_SPECIFICATIONS = 64;

    /**
     * dws table集的最大
     */
    public static final int TABLE_SPECIFICATIONS = 256;

    /**
     * 查询数据库中表/schema集的规格
     */
    public static final int QUERY_QUANTITY_SPECIFICATIONS = 100;

    /**
     * 资源不支持恢复
     */
    public static final String DATABASE_RESTORE_FAIL_NOT_ALLOW_LABEL = "database_restore_not_allow_restore_label";

    /**
     * 判断是否继续往后走类型
     */
    public static final Map<PolicyAction, String> SLA_ACTION = ImmutableMap.of(
        PolicyAction.DIFFERENCE_INCREMENT, "common_incremental_backup_label",
        PolicyAction.CUMULATIVE_INCREMENT, "common_diff_backup_label");

    /**
     * DWS_CLUSTER_STATUS 转换值
     */
    public static final Map<String, String> DWS_CLUSTER_STATUS = ImmutableMap.of(
        DwsConstant.EXTEND_INFO_NORMAL_VALUE_STATE, LinkStatusEnum.ONLINE.getStatus().toString(),
        DwsConstant.EXTEND_INFO_UNAVAILABLE_VALUE_STATE, LinkStatusEnum.UNAVAILABLE.getStatus().toString(),
        DwsConstant.EXTEND_INFO_DEGRADED_VALUE_STATE, LinkStatusEnum.DEGRADED.getStatus().toString(),
        DwsConstant.EXTEND_INFO_UNSTARTED_VALUE_STATE, LinkStatusEnum.UNSTARTED.getStatus().toString());
}
