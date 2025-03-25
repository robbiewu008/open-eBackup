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
package openbackup.database.base.plugin.common;

/**
 * 通用数据库常量
 *
 */
public class GeneralDbConstant {
    /**
     * 集群信息key
     */
    public static final String CLUSTER_FULL_INFO = "clusterFullInfo";

    /**
     * 集群节点dependency key
     */
    public static final String DEPENDENCY_CLUSTER_NODE_KEY = "children";

    /**
     * 主机dependency key
     */
    public static final String DEPENDENCY_HOST_KEY = "hosts";

    /**
     * 部署类型-key
     */
    public static final String EXTEND_DEPLOY_TYPE = "deployType";

    /**
     * 通用数据库类型-key
     */
    public static final String EXTEND_FIRST_CLASSIFICATION_KEY = "firstClassification";

    /**
     * 通用数据库实例类型-key
     */
    public static final String EXTEND_INSTANCE_TYPE_KEY = "instanceType";

    /**
     * 通用数据库脚本-key
     */
    public static final String EXTEND_SCRIPT_KEY = "script";

    /**
     * 自定义参数-key
     */
    public static final String EXTEND_CUSTOM_PARAM = "customParams";

    /**
     * 自定义参数-长度限制
     */
    public static final int EXTEND_CUSTOM_PARAM_LENGTH = 500;

    /**
     * 脚本配置-key
     */
    public static final String EXTEND_SCRIPT_CONF = "scriptConf";

    /**
     * 通用数据库类型-实例
     */
    public static final String GENERAL_DB_INSTANCE = "1";

    /**
     * 通用数据库类型-数据库
     */
    public static final String GENERAL_DB_DATABASE = "2";

    /**
     * 通用数据库实例类型-单机
     */
    public static final String INSTANCE_SINGLE = "1";

    /**
     * 通用数据库实例类型-集群
     */
    public static final String INSTANCE_CLUSTER = "2";

    /**
     * 副本时间点恢复的时候时间点参数key
     */
    public static final String RESTORE_TIME_STAMP_KEY = "restoreTimestamp";

    /**
     * 任务时间点恢复KEY
     */
    public static final String RESTORE_ANY_TIME_POINT = "pirt";

    /**
     * 用于表示是否使用下个主机的support接口
     */
    public static final String EXTEND_SHOULD_NEXT_SUPPORT_KEY = "shouldNextSupport";

    /**
     * 所有主机IP，extend key
     */
    public static final String EXTEND_RELATED_HOST_IPS = "relatedHostIps";

    /**
     * 所有主机ID，extend key
     */
    public static final String EXTEND_RELATED_HOST_IDS = "relatedHostIds";

    /**
     * 数据库具体类型的展示名称
     */
    public static final String DATABASE_TYPE_DISPLAY = "databaseTypeDisplay";

    /**
     * 应用version key
     */
    public static final String EXTEND_VERSION_KEY = "applicationVersion";

    /**
     * 扩展信息，原始资源key
     */
    public static final String EXTEND_ORIGIN_RESOURCE_KEY = "originProtectedResource";

    /**
     * SAP HANA 数据库具体类型的展示名称
     */
    public static final String DATABASE_TYPE_DISPLAY_SAP_HANA = "SAP HANA";

    /**
     * SAP HANA System ID key
     */
    public static final String SYSTEM_ID = "systemId";

    /**
     * SAP HANA实例所有节点列表key
     */
    public static final String NODES = "nodes";

    /**
     * 通用数据库脚本 值 SAP HANA
     */
    public static final String EXTEND_SCRIPT_VAL_SAPHANA = "saphana";

    private GeneralDbConstant() {
    }
}
