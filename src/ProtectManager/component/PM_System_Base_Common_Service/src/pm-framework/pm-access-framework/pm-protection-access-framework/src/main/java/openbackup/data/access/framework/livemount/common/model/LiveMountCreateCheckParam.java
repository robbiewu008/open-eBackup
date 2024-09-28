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

import openbackup.data.access.framework.livemount.common.enums.OperationEnums;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyResourceSummary;
import openbackup.system.base.sdk.resource.model.ResourceEntity;

import lombok.Data;

import java.util.List;

/**
 * Live Mount Create Check Param
 *
 */
@Data
public class LiveMountCreateCheckParam {
    private LiveMountObject liveMountObject;

    private CopyResourceSummary resource;

    private List<ResourceEntity> targetResources;

    private Copy copy;

    private OperationEnums operationEnums;
}
