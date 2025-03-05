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
package openbackup.system.base.sdk.devicemanager.request;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

import javax.validation.Valid;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

/**
 * 更新pacific 集群业务网络配置请求体
 *
 */
@Getter
@Setter
public class UpdateBusinessNetworkRequest {
    @Valid
    @NotNull
    @Size(min = 1, message = "The backup network pacific network info is configured with at least one property")
    private List<NodeNetworkInfoRequest> backupNetWorkInfoList;

    @Valid
    private List<NodeNetworkInfoRequest> archiveNetWorkInfoList;

    @Valid
    private List<NodeNetworkInfoRequest> copyNetWorkInfoList;
}
