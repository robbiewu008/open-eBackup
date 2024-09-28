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

import com.fasterxml.jackson.core.type.TypeReference;

/**
 * 资源扩展属性handler
 *
 */
public abstract class ResourceExtensionHandler<T, R> {
    /**
     * 默认构造
     */
    protected ResourceExtensionHandler() {
        new TypeReference<T>() {};
        new TypeReference<R>() {};
    }

    /**
     * 配置的路径
     *
     * @return namePath
     */
    public abstract String getNamePath();

    /**
     * 对扩展资源的处理
     *
     * @param configObj 配置对象
     * @param params params
     * @return R
     */
    public abstract R handle(Object configObj, T params);
}
