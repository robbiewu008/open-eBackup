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
package com.huawei.oceanprotect.k8s.protection.access.constant;

/**
 * 功能描述: K8sConstant
 *
 * @author l00570077
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-18
 */
public class K8sConstant {
    /**
     * Agent 返回的成功错误码
     */
    public static final String SUCCESS = "0";

    /**
     * 查询集群信息，version key
     */
    public static final String CLUSTER_VERSION = "version";

    /**
     * k8s 最低可接受版本 1.17
     */
    public static final String K8S_MIN_VERSION = "v1.17";

    /**
     * namespace下dataset最大数量
     */
    public static final int DATASET_MAX_COUNT_IN_NAMESPACE = 16;

    /**
     * Tag要求以字母或数字开头和结尾,由字母、数字、”_"、"."或"-"组成,不得超过63个字符
     */
    public static final String TAG_PATTERN = "^[a-zA-Z0-9][a-zA-Z0-9_.-]{0,61}[a-zA-Z0-9]$|^[a-zA-Z0-9]$";


    /**
     * k8s容器的部署类型
     */
    public static final String DEPLOY_TYPE = "deployType";

    /**
     * 部署类型为分布式
     */
    public static final String DISTRIBUTED = "5";

    /**
     * 源备份副本的uuid
     */
    public static final String ORIGIN_BACKUP_ID = "originBackupId";

    /**
     * 是否开启修改环境变量开关变量的名称
     */
    public static final String IS_ENABLE_CHANGE_ENV = "isEnableChangeEnv";

    /**
     * 修改环境变量具体参数列表的名称
     */
    public static final String ADVANCED_CONFIG_REQ_LIST = "advancedConfigReqList";

    /**
     * 是否开启修改Sc开关变量的名称
     */
    public static final String IS_ENABLE_CHANGE_SC_PARAMETER = "isEnableChangScParameter";

    /**
     * 修改Sc变量具体参数列表的名称
     */
    public static final String SC_PARAMETER_LIST = "scParameterList";

    /**
     * 修改环境变量列表 参数最少个数 >0
     */
    public static final int ADVANCED_CONFIG_MIN_SIZE = 0;

    /**
     * 修改环境变量列表 参数最大个数 <= 10
     */
    public static final int ADVANCED_CONFIG_MAX_SIZE = 10;

    /**
     * 环境变量最小长度
     */
    public static final int ADVANCED_CONFIG_PARAM_MIN_SIZE = 1;

    /**
     * 环境变量最大长度
     */
    public static final int ADVANCED_CONFIG_PARAM_MAX_SIZE = 100;
}