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
package openbackup.data.access.client.sdk.api.framework.dme.model;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;
import openbackup.data.access.client.sdk.api.framework.dme.DmeMountQos;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.system.base.sdk.resource.model.CifsProtocol;
import openbackup.system.base.sdk.resource.model.NfsProtocol;

import java.util.List;

/**
 * 功能描述
 *
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@Builder
public class AgentLessActionRequest {
    private StorageRepository repository;

    private NfsProtocol nfsProtocol;

    private CifsProtocol cifsProtocol;

    private String filesystemName;

    private DmeMountQos qos;

    private List<VpcInfo> vpcInfos;
}
