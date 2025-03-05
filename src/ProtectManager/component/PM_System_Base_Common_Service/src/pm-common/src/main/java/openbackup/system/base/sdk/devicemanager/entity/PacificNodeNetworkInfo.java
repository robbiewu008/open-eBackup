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

/**
 * pacific节点业务网络信息
 *
 */
@Getter
@Setter
public class PacificNodeNetworkInfo {
    // 备份业务网络信息
    private List<PacificIpInfo> backupIpInfoList;

    // 归档业务网络信息
    private List<PacificIpInfo> archiveIpInfoList;

    // 复制业务网络信息
    private List<PacificIpInfo> replicationIpInfoList;
}
