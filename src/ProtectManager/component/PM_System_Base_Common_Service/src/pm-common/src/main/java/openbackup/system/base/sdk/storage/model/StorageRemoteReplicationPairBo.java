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
package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 本地远程复制Pair
 *
 * @author g30003063
 * @since 2021/12/14
 */
@Getter
@Setter
public class StorageRemoteReplicationPairBo {
    /**
     * 远程复制PairID
     */
    @JsonProperty("ID")
    private String id;

    /**
     * 本端是否是主端
     */
    @JsonProperty("ISPRIMARY")
    private boolean isPrimary;

    /**
     * 复制模式
     * 1：同步
     * 2：异步
     */
    @JsonProperty("REPLICATIONMODEL")
    private int replicationModel;
}
