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
 * OpenGauss相关错误码
 *
 */
public class OpenGaussErrorCode {
    /**
     * 错误场景：执行注册/修改应用集群操作时，由于选择的集群类型与应用集群类型不匹配，操作失败。
     * 原因：选择的集群类型与应用集群类型不匹配。
     * 建议：请选择与集群类型相匹配的应用后重试。
     */
    public static final long CLUSTER_CLUSTER_TYPE_INCONSISTENT = 1577209995L;

    /**
     * 场景：注册/修改应用集群操作，检测注册信息成功。
     */
    public static final long SUCCESS = 0L;
}
