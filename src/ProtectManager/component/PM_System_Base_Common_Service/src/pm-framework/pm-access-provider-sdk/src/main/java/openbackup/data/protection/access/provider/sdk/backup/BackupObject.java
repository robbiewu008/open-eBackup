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
package openbackup.data.protection.access.provider.sdk.backup;

import openbackup.data.protection.access.provider.sdk.base.Parameter;
import openbackup.data.protection.access.provider.sdk.sla.Policy;
import openbackup.data.protection.access.provider.sdk.sla.Sla;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * Backup Object
 *
 */
@Data
public class BackupObject {
    @JsonProperty("request_id")
    private String requestId;

    @JsonProperty("task_id")
    private String taskId;

    @JsonProperty("chain_id")
    private String chainId;

    @JsonProperty("backup_type")
    private String backupType;

    @JsonProperty("protected_object")
    private ProtectedObject protectedObject;

    @JsonProperty("sla")
    private Sla sla;

    private Repository repository;

    private List<Parameter> parameters;

    private Policy policy;
}
