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
package openbackup.data.access.framework.livemount.common.model;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.sdk.copy.model.Copy;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

/**
 * Live Mount Execute Param
 *
 * @author l00272247
 * @since 2020-09-22
 */
@Data
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class LiveMountExecuteParam {
    private String requestId;

    private String jobId;

    private LiveMountEntity liveMount;

    private Copy cloneCopy;

    private Copy sourceCopy;

    private Copy mountedCopy;

    private ProtectedResource targetResource;
}
