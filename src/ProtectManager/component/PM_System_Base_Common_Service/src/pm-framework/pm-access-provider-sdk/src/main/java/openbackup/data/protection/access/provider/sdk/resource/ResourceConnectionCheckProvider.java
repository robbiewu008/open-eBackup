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
package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.util.ResourceCheckContextUtil;

/**
 * 资源连通性检查provider
 *
 */
public interface ResourceConnectionCheckProvider extends DataProtectionProvider<ProtectedResource> {
    /**
     * 检查资源连通性
     * 该方法不会抛出异常，需要对返回结果进行检查
     *
     * @param protectedResource 受保护资源
     * @param protectedResourceChecker 检查checker
     * @return 检查结果
     */
    ResourceCheckContext tryCheckConnection(ProtectedResource protectedResource,
        ProtectedResourceChecker protectedResourceChecker);

    /**
     * 检查资源连通性
     * 该方法不会抛出异常，需要对返回结果进行检查
     *
     * @param protectedResource 受保护资源
     * @return 检查结果
     */
    default ResourceCheckContext tryCheckConnection(ProtectedResource protectedResource) {
        return tryCheckConnection(protectedResource, null);
    }

    /**
     * 检查资源连通性
     *
     * @param protectedResource 受保护资源
     * @return 检查结果
     */
    default ResourceCheckContext checkConnection(ProtectedResource protectedResource) {
        ResourceCheckContext context = tryCheckConnection(protectedResource, null);
        ResourceCheckContextUtil.check(context, "check connection failed.");
        return context;
    }
}