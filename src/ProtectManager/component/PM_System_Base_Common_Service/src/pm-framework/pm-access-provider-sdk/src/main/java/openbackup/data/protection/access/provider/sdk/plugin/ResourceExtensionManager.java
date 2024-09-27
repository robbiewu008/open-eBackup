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
package openbackup.data.protection.access.provider.sdk.plugin;

/**
 * 资源扩展invoke管理
 *
 * @author h30027154
 * @since 2022-05-30
 */
public interface ResourceExtensionManager {
    /**
     * invoke
     *
     * @param subType subType
     * @param namePath 配置的路径
     * @param <T> 输入类型
     * @param <R> 输出类型
     * @param context context
     * @return R
     */
    <T, R> R invoke(String subType, String namePath, PluginExtensionInvokeContext<T, R> context);
}
