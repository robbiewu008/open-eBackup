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
package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;

/**
 * Global Search index
 *
 **/
@Getter
@Setter
public class ScanRequest extends SearchBaseMsg {
    @JsonProperty("snap_info")
    private SnapInfo snapInfo;

    @JsonProperty("storage_info")
    private RestoreStorageInfo storageInfo;

    @JsonProperty("storage_repository")
    private StorageRepository storageRepository;
}
