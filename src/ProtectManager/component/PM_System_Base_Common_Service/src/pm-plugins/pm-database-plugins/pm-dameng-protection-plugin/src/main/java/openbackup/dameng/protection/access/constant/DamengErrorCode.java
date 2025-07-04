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
package openbackup.dameng.protection.access.constant;

/**
 * dameng相关错误码定义
 *
 */
public class DamengErrorCode {
    /**
     * dameng注册的实例和集群实例数量不一致
     */
    public static final long INSTANCES_NUMBER_DIFF = -1L;

    /**
     * dameng注册的实例不属于同一集群
     */
    public static final long INSTANCES_CLUSTER_DIFF = -2L;

    /**
     * dameng集群实例的主备关系错误
     */
    public static final long INSTANCES_PRIMARY_SECONDARY_ERROR = -3L;
}
