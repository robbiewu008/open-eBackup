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

import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * VmwareFLRRequestBody
 *
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class VmFlrRequest {
    private String jobId;

    private String requestId;

    private String defaultPublishTopic;

    private String responseTopic;

    private String userId;

    private String snapMetadata;

    private String snapId;

    private String indexed;

    private RestoreStorageInfo storageInfo;

    private List<String> paths;

    private VmFlrDestInfo destInfo;

    private String replaceMode;

    private String snapType;

    private String recordId;

    private String resourceSubType;

    /**
     * 设备Id
     */
    private String deviceId;

    /**
     * 存储单元id
     */
    private String storageId;
}
