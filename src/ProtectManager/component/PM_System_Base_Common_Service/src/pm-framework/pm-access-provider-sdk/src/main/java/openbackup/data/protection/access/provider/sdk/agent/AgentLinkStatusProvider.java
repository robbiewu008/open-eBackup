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
package openbackup.data.protection.access.provider.sdk.agent;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import java.util.Arrays;
import java.util.List;

/**
 * 根据资源选择获取连接状态排序列表
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-08-04
 */
public interface AgentLinkStatusProvider extends DataProtectionProvider<ProtectedResource> {
    /**
     * 获取agent的状态优先级排序
     *
     * @return agent的连接状态优先级排序
     */
    default List<LinkStatusEnum> getLinkStatusOrderList() {
        return Arrays.asList(LinkStatusEnum.ONLINE, LinkStatusEnum.OFFLINE);
    }
}
