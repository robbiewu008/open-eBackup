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
package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.sdk.cluster.enums.ClusterEnum;

import com.fasterxml.jackson.annotation.JsonIgnore;

import lombok.Data;
import lombok.NoArgsConstructor;

import org.hibernate.validator.constraints.Length;

/**
 * Clusters relation request
 *
 */
@Data
@NoArgsConstructor
public class ClusterRelationRequest {
    private static final int MIN_ESN_LEN = 1;

    private static final int MAX_ESN_LEN = 256;

    private ClusterEnum.OperateType operateType;

    @Length(min = MIN_ESN_LEN, max = MAX_ESN_LEN)
    private String sourceClusterEsn;

    /**
     * 操作类型字符描述，包括三种状态（增加、修改、删除），rest接口中需要忽略。
     */
    @JsonIgnore
    private String operateTypeDesc;

    /**
     * 构造器
     *
     * @param type 操作类型
     * @param esn 源端集群ESN
     */
    public ClusterRelationRequest(ClusterEnum.OperateType type, String esn) {
        this.operateType = type;
        this.sourceClusterEsn = esn;
    }
}
