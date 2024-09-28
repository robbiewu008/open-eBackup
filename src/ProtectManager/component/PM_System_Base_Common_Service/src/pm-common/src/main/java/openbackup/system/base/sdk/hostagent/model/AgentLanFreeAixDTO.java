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
package openbackup.system.base.sdk.hostagent.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

import javax.validation.Valid;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

/**
 * AIX主机 LAN-FREE 配置页信息
 *
 */
@Getter
@Setter
public class AgentLanFreeAixDTO {
    @NotNull
    private boolean isDelete;

    /**
     * 选择的数据协议 FC/ISCSI
     */
    @Pattern(regexp = "FC|ISCSI")
    private String dataProtocol = "FC";

    /**
     * 选择的SanClient主机id
     */
    @NotNull
    private List<String> sanclientResourceIds;

    /**
     * AIX主机 WWPN信息
     */
    @Valid
    private List<AgentLanFreeWwpn> clientWwpns;

    /**
     * AIX主机 IQN信息
     */
    private List<String> clientIqns;
}
