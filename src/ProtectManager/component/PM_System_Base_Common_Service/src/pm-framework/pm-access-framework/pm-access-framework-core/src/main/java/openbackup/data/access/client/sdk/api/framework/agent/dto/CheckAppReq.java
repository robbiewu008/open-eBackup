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
package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.util.BeanTools;

/**
 * Check App Req
 *
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class CheckAppReq {
    private AppEnv appEnv;

    private Application application;

    /**
     * 根据受保护资源或环境信息构造CheckAppReq
     *
     * @param resource 受保护资源或环境
     * @return CheckAppReq
     */
    public static CheckAppReq buildFrom(ProtectedResource resource) {
        AppEnv tempAppEnv = BeanTools.copy(resource, AppEnv::new);
        Application tempApplication = BeanTools.copy(resource, Application::new);
        return new CheckAppReq(tempAppEnv, tempApplication);
    }
}