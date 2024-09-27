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
package openbackup.system.base.sdk.cluster.request;

import openbackup.system.base.sdk.cluster.model.ClusterComponentPwdInfo;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 功能描述
 *
 * @author c30047317
 * @since 2023-07-27
 */
@Data
public class ClusterComponentPwdInfoRequest {
    /**
     * 内部组件密码信息
     */
    private List<ClusterComponentPwdInfo> clusterComponentPwdInfoList;

    /**
     * 是否正在组建多集群
     */
    @JsonProperty("isAssemble")
    private boolean isAssemble;

    /**
     * 节点角色
     */
    @JsonProperty("roleType")
    private int roleType = 0;
}
