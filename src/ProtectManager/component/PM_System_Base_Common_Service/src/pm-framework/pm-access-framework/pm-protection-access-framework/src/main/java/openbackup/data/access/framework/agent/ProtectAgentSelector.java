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
package openbackup.data.access.framework.agent;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;
import java.util.Map;

/**
 * 保护代理提供者接口定义，通过该接口
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-26
 */
public interface ProtectAgentSelector extends DataProtectionProvider<String> {
    /**
     * 选择备份/备份恢复代理
     *
     * @param protectedResource 备份/恢复资源{@link ProtectedResource}
     * @param parameters 备份恢复参数
     * @return 返回备份/恢复代理
     */
    List<Endpoint> select(ProtectedResource protectedResource, Map<String, String> parameters);
}
