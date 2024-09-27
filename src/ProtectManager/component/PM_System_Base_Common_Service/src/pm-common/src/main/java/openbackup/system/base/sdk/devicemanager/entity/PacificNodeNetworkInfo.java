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
package openbackup.system.base.sdk.devicemanager.entity;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

import javax.annotation.Nullable;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

/**
 * pacific节点业务网络信息
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-01
 */
@Getter
@Setter
public class PacificNodeNetworkInfo {
    // 备份业务网络信息
    @NotNull
    @Size(min = 1)
    private List<PacificIpInfo> backupIpInfoList;

    @NotNull
    // 归档业务网络信息
    private List<PacificIpInfo> archiveIpInfoList;

    @Nullable
    private List<PacificIpInfo> replicationIpInfoList;
}
