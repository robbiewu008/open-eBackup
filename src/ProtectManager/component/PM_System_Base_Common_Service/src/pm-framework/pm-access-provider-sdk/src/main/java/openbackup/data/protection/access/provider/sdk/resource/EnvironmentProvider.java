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
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.exception.NotImplementedException;

import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * 受保护环境Provider接口定义，该接口提供受保护环境资源扫描，健康状态检查，资源浏览，环境信息检查标准接口
 *
 */
public interface EnvironmentProvider extends DataProtectionProvider<String> {
    /**
     * 扫描受保护环境， 可选实现。根据受保护保护环境决定是否实现该接口，
     * 如受保护的资源需要在ProtectManager进行持久化，则需实现，如VMware虚拟化环境；
     * 如不需要将资源在ProtectManager中实现，则无须实现，比如HDFS的目录、文件，HBase的命名空间，表等。
     *
     * @param environment 受保护环境
     * @return 受保护环境中的资源列表
     */
    default List<ProtectedResource> scan(ProtectedEnvironment environment) {
        throw new NotImplementedException("no need to scan");
    }

    /**
     * 对环境信息进行检查，该接口用于注册受保护环境，修改受保护环境时对环境信息进行验证
     * 比如检查受保护环境与ProtectManager之间的连通性，认证信息是否合法。环境参数是否合法等等
     * 检查不通过抛出com.huawei.oceanprotect.data.protection.access.provider.sdk.exception.DataProtectionAccessException
     *
     * @param environment 受保护环境
     */
    void register(ProtectedEnvironment environment);

    /**
     * 浏览环境资源，可选实现：
     * 对于大数据平台受保护环境，其上的受保护资源不会在ProtectManager中进行持久化。需要实时层级浏览目录，文件，
     * 并为这些目录创建逻辑资源，如文件集，则需要实现。
     *
     * @param environment 受保护环境
     * @param environmentConditions 查询资源的条件
     * @return 返回资源列表
     */
    default PageListResponse<ProtectedResource> browse(
        ProtectedEnvironment environment, BrowseEnvironmentResourceConditions environmentConditions) {
        return new PageListResponse<>(0, Collections.emptyList());
    }

    /**
     * 受保护环境健康状态检查，
     * 状态异常抛出com.huawei.oceanprotect.data.protection.access.provider.sdk.exception.DataProtectionAccessException，
     * 并返回具体的错误码
     *
     * @param environment 受保护环境
     */
    void validate(ProtectedEnvironment environment);

    /**
     * 受保护环境健康状态检查, 返回连接状态
     *
     * @param environment 受保护环境
     * @return LinkStatusEnum
     */
    default Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment) {
        validate(environment);
        return Optional.empty();
    }

    /**
     * 移除受保护环境
     *
     * @param environment 受保护环境
     */
    default void remove(ProtectedEnvironment environment) {
    }
}
