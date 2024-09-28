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
package openbackup.data.access.client.sdk.api.framework.agent.dto.model;

import lombok.Data;

/**
 * 调用agent查询到的AIX主机和SanClient主机WWPN或IQN信息类
 *
 */
@Data
public class WwpnOrIqnInfo {
    /**
     * WWPN值或IQN值
     */
    private String configKey;

    /**
     * 在线状态 27在线，28离线
     */
    private String configValue;
}
