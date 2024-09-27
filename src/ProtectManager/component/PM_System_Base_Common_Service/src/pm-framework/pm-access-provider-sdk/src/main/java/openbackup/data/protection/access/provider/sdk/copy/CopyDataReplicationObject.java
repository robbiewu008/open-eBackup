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
package openbackup.data.protection.access.provider.sdk.copy;

import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Copy Data Replication Object
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-17
 */
@Data
public class CopyDataReplicationObject {
    @JsonProperty("copy_id")
    private ProtectedObject protectedObject;

    @JsonProperty("replication_target")
    private CopyDataReplicationTarget replicationTarget;
}
