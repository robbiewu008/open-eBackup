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

import lombok.Data;

/**
 * 资源特性
 *
 * @author h30027154
 * @since 2022-07-04
 */
@Data
public class ResourceFeature {
    // 检查环境名称是否重复
    private boolean shouldCheckEnvironmentNameDuplicate = true;

    // 检查资源名称是否重复
    private boolean shouldCheckResourceNameDuplicate = true;

    // 扫描资源直接入库
    private boolean shouldSaveDirectlyWhenScan = false;

    // 是否在扫描时更新dependency的主机信息
    private boolean shouldUpdateDependencyHostInfoWhenScan = true;

    // 应用是否支持lanfree配置，默认支持
    private boolean isSupportedLanFree = true;

    /**
     * 默认值
     *
     * @return ResourceFeature
     */
    public static ResourceFeature defaultValue() {
        return new ResourceFeature();
    }
}
