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
package openbackup.tdsql.resources.access.constant;

/**
 * 功能描述 TDSQL常量
 *
 */
public class TdsqlConstant {
    /**
     * TDSQL
     */
    public static final String TDSQL = "TDSQL";

    /**
     * 取json里面的clusterInfo
     */
    public static final String CLUSTER_INFO = "clusterInfo";

    /**
     * 取json里面的clusterInstanceInfo
     */
    public static final String CLUSTER_INSTANCE_INFO = "clusterInstanceInfo";

    /**
     * 取json里面的clusterGroupInfo
     */
    public static final String CLUSTER_GROUP_INFO = "clusterGroupInfo";

    /**
     * 参数
     */
    public static final String PARAMETERS = "parameters";

    /**
     * 截取node长度
     */
    public static final int SUB_NODE_LENGTH = 4;

    /**
     * subType
     */
    public static final String SUBTYPE = "subType";

    /**
     * 父资源uuid
     */
    public static final String PARENT_UUID = "parentUuid";

    /**
     * OSS
     */
    public static final String OSS_NODE_TYPE = "ossNode";

    /**
     * SCHEDULER
     */
    public static final String SCHEDULER_NODE_TYPE = "schedulerNode";

    /**
     * data node
     */
    public static final String DATA_NODE_TYPE = "dataNode";

    /**
     * 实例id
     */
    public static final String INSTANCE_ID = "id";

    /**
     * check type: check_manage_node_num / check_node
     */
    public static final String CHECK_TYPE = "checkType";

    /**
     * check_node
     */
    public static final String CHECK_NODE = "check_node";

    /**
     * check_group_data_node
     */
    public static final String CHECK_GROUP_DATA_NODE = "check_group_data_node";

    /**
     * check_group_info
     */
    public static final String CHECK_GROUP_INFO = "check_group_info";

    /**
     * requestUrl
     */
    public static final String REQUESTURL = "requestUrl";

    /**
     * oss
     */
    public static final String OSS = "oss";

    /**
     * oss
     */
    public static final String SCHEDULER = "scheduler";

    /**
     * singleNode
     */
    public static final String SINGLENODE = "singleNode";

    /**
     * dataNode
     */
    public static final String DATANODE = "dataNode";

    /**
     * groupInfo
     */
    public static final String GROUP = "group";

    /**
     * linkStatus
     */
    public static final String LINKSTATUS = "linkStatus";

    /**
     * TDSQL-clusterInstance
     */
    public static final String TDSQL_CLUSTERINSTACE = "TDSQL-clusterInstance";

    /**
     * TDSQL-clusterGroup
     */
    public static final String TDSQL_CLUSTER_GROUP = "TDSQL-clusterGroup";

    /**
     * TDSQL-cluster
     */
    public static final String TDSQL_CLUSTER = "TDSQL-cluster";

    /**
     * UBackupAgent
     */
    public static final String U_BACKUP_AGENT = "UBackupAgent";

    /**
     * instanceType
     */
    public static final String INSTANCE_TYPE = "instanceType";

    /**
     * id
     */
    public static final String ID = "id";

    /**
     * ossUrl
     */
    public static final String OSS_URL = "ossUrl";

    /**
     * semicolon 分号
     */
    public static final String SEMICOLON = ";";

    /**
     * comma 逗号
     */
    public static final String COMMA = ",";

    /**
     * 日志副本时间点恢复的时候时间点参数
     */
    public static final String RESTORE_TIME_STAMP_KEY = "restoreTimestamp";

    /**
     * 错误场景：执行注册/修改应用集群操作时，由于选择的集群类型与应用集群类型不匹配，操作失败。
     * 原因：选择的集群类型与应用集群类型不匹配。
     * 建议：请选择与集群类型相匹配的应用后重试
     */
    public static final long CHECK_TDSQL_DEPLOYMENT_MODEL_FAILED = 1577209995L;

    /**
     * 错误场景：执行校验数据库集群操作时，由于集群信息填写错误，操作失败。
     * 原因：集群信息填写错误。
     * 建议：请确保集群信息正确。
     */
    public static final long CHECK_CLUSTER_FAILED = 1577209956L;

    /**
     * 错误场景：执行TDSQL实例备份操作时，由于添加的实例节点个数与实际的实例节点个数不一致，操作失败。
     * 原因：添加的实例节点个数与实际的实例节点个数不一致。
     * 建议：请确保注册的实例节点个数与TDSQL赤兔管理台查询的实例节点个数一致。
     */
    public static final long INCONSISTENT_NODES = 1677873230L;

    /**
     * 错误场景：执行注册TDSQL实例操作时，由于数据节点信息未包含该实例下所有的数据节点，操作失败。
     * 原因：数据节点信息未包含该实例下所有的数据节点。
     * 建议：请检查配置的数据节点的个数。
     */
    public static final long MISSING_DATA_NODE = 1677873228L;

    /**
     * 错误场景：执行注册TDSQL实例操作时，由于未查询到所填实例扫描出的数据节点信息，操作失败。
     * 原因：未查询到所填实例扫描出的数据节点信息。
     * 建议：请确保所填实例信息能扫描出该实例下所有数据节点信息。
     */
    public static final long NO_DATA_NODE_INFO = 1677873229L;

    /**
     * 错误场景：执行TDSQL实例备份操作时，由于TDSQL实例不存在，操作失败。
     * 原因：实例不存在。
     * 建议：请在TDSQL赤兔管理台检查注册的实例是否存在。
     */
    public static final long NO_INSTANCE_EXISTS = 1677873231L;

    /**
     * 插件指定PM需要捕获的错误码
     * 原因：数据库存在未运行的服务（[{0}]）。
     * 建议：请在数据库所有服务都运行后重试。
     */
    public static final long MISS_COMPONENT = 1577213479L;

    /**
     * 插件指定PM需要捕获的错误码
     * 执行注册\修改GoldenDB集群或集群实例操作时，由于添加的集群节点类型错误，操作失败。
     * 原因：添加的集群节点类型错误。
     * 建议：请确保在（{0}）节点下，输入主机（{1}）用户名({2})的节点类型为（{3}）。
     */
    public static final long NODE_TYPE_MISMATCH = 1577209938L;

    /**
     * check instance data node failed.
     */
    public static final long CONNECT_TO_DB_ERROR = 1677873202L;

    /**
     * 即时挂载前置脚本
     */
    public static final String PRE_SCRIPT = "pre_script";

    /**
     * 即时挂载后置脚本
     */
    public static final String POST_SCRIPT = "post_script";

    /**
     * 即时失败后置脚本
     */
    public static final String FAIL_POST_SCRIPT = "failed_script";

    /**
     * set分片在线状态
     */
    public static final String SET_IS_ONLINE = "0";

    /**
     * TDSQL持续挂载key
     */
    public static final String PERSISTENT_MOUNT = "persistentMount";

    /**
     * TDSQL手动挂载key
     */
    public static final String MANUAL_MOUNT = "manualMount";

    /**
     * 是否需要删除Dtree
     */
    public static final String NEED_DELETE_DTREE = "needDeleteDtree";

    /**
     * queryType
     */
    public static final String QUERY_TYPE_KEY = "queryType";

    /**
     * MACHINE 查询实例机型
     */
    public static final String MACHINE = "machine";

    /**
     * RESOURCE 查询集群数据节点
     */
    public static final String RESOURCE = "resource";
}
