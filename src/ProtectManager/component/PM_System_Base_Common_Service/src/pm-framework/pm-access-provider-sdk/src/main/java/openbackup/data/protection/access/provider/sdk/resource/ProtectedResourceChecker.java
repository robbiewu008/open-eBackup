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

import java.util.List;
import java.util.Map;

/**
 * The ResourceConnectionChecker
 *
 */
public interface ProtectedResourceChecker<T> extends DataProtectionProvider<ProtectedResource> {
    /**
     * 获取受保护资源环境矩阵
     *
     * @param resource 需要检查的受保护资源
     * @return 需要检查的资源集合
     */
    Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource);

    /**
     * 检查连通性
     *
     * @param protectedResource 需要检查的资源
     * @return 检查报告
     */
    CheckResult<T> generateCheckResult(ProtectedResource protectedResource);

    /**
     * 获取检查结果
     *
     * @param checkReports 检查报告
     * @param context 上下文
     * @return 检查结果
     */
    List<ActionResult> collectActionResults(List<CheckReport<T>> checkReports, Map<String, Object> context);
}
