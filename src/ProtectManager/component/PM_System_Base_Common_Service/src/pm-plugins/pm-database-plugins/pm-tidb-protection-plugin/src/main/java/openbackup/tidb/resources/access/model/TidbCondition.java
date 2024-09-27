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
package openbackup.tidb.resources.access.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * tidb Condition
 *
 * @author w00426202
 * @since 2023-07-15
 */
@Setter
@Getter
public class TidbCondition {
    /**
     * 操作类型
     */
    @JsonProperty("action_type")
    private String actionType;

    /**
     * 集群名称
     */
    private String clusterName;

    /**
     * 是否集群
     */
    private boolean isCluster;

    /**
     * 主机列表
     */
    private List<String> agentIds;
}
