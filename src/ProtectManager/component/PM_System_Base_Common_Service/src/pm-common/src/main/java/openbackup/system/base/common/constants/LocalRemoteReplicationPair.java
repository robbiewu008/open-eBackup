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
package openbackup.system.base.common.constants;

import lombok.Getter;
import lombok.Setter;

/**
 * 本地远程复制Pir
 *
 */
@Getter
@Setter
public class LocalRemoteReplicationPair {
    /**
     * 远程复制PairID
     */
    private String id;

    /**
     * 本端是否是主端
     */
    private boolean isPrimary;

    /**
     * 复制模式
     * 1：同步
     * 2：异步
     */
    private int replicationModel;
}
