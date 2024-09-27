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
package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * SaveImportCopyParam
 *
 * @author g00500588
 * @since 2021/9/15
 */
@Data
public class SaveImportCopyParam {
    private String uuid;

    private String name;

    @JsonProperty("generated_time")
    private long generatedTime;

    @JsonProperty("generated_by")
    private String generatedBy;

    @JsonProperty("resource_type")
    private String resourceType;

    @JsonProperty("resource_sub_type")
    private String resourceSubType;

    @JsonProperty("resource_id")
    private String resourceId;

    @JsonProperty("chain_id")
    private String chainId;
}
