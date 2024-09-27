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
package openbackup.data.protection.access.provider.sdk.livemount;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.system.base.common.constants.LegoNumberConstant;

/**
 * Live Mount Interceptor Provider
 *
 * @author l00272247
 * @since 2021-12-29
 */
public interface LiveMountInterceptorProvider extends DataProtectionProvider<String> {
    /**
     * 标记资源是否支持刷新目标环境，针对没有目标环境的场景，将该字段标记为false。<br/>
     * 没有目标环境的场景，挂载之后LiveMount的挂载资源ID为空。<br/>
     *
     * @return 支持刷新目标环境的标记
     */
    default boolean isRefreshTargetEnvironment() {
        return true;
    }

    /**
     * 标记资源是否支持在相同目标环境上重复挂载
     *
     * @return 重复标记
     */
    default boolean isSupportRepeatable() {
        return false;
    }

    /**
     * 是否支持检查该副本的操作权限
     *
     * @return 是否支持
     */
    default boolean isSupportCheckCopyOperation() {
        return false;
    }

    /**
     * init live mount create param
     *
     * @param task task
     */
    void initialize(LiveMountCreateTask task);

    /**
     * init live mount cancel task param
     *
     * @param task task
     */
    void finalize(LiveMountCancelTask task);

    /**
     * 获取同资源同位置可挂载数量上限
     *
     * @return 挂载数量上限
     */
    default Integer getLiveMountNumLimit() {
        return LegoNumberConstant.ONE;
    }
}
