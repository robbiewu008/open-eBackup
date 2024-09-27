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
package openbackup.oceanbase.common.constants;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-21
 */
public interface OBErrorCodeConstants {
    /**
     * 认证信息错误
     */
    long AUTH_ERROR = 1577209942L;

    /**
     * 集群状态异常
     */
    long CLUSTER_STATUS_INVALID_ERROR = 1577213524L;

    /**
     * 集群节点数不一致
     */
    long CLUSTER_NODE_COUNT_NOT_SAME_ERROR = 1577209972L;

    /**
     * OBServer不属于同一个集群
     */
    long OBSERVER_IS_NOT_ONE_CLUSTER_ERROR = 1577213525L;

    /**
     * OBServer连接异常
     */
    long OBSERVER_CONNECT_ERROR = 1577213522L;

    /**
     * 租户不存在（注册租户集）
     */
    long TENANT_NOT_EXIST = 1577213527L;

    /**
     * OBServer的业务IP和agent的IP不匹配
     */
    long OBSERVER_IP_NOT_MATCH = 1677947141L;
}
