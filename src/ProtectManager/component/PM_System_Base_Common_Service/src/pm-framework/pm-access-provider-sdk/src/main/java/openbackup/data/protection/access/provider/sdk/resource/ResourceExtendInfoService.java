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

import openbackup.data.protection.access.provider.sdk.resource.model.ProtectedResourceExtendInfo;

import java.util.List;
import java.util.Map;

/**
 * 资源扩展表服务
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/7/13
 */
public interface ResourceExtendInfoService {
    /**
     * 删除资源的扩展信息
     *
     * @param resourceId 资源ID
     * @param keys 扩展表key，该值为空，则全删
     */
    void deleteByKeys(String resourceId, String... keys);

    /**
     * 保存或者更新资源的扩展信息
     *
     * @param resourceId 资源ID
     * @param extendInfoMap 扩展信息
     */
    void saveOrUpdateExtendInfo(String resourceId, Map<String, String> extendInfoMap);

    /**
     * 查询资源的扩展信息
     *
     * @param resourceId 资源ID
     * @param keys 扩展表key，该值为空，则全查
     * @return 扩展信息，无值返回空map
     */
    Map<String, String> queryExtendInfo(String resourceId, String... keys);

    /**
     * 查询资源的扩展信息
     *
     * @param resourceIds 资源ID集合
     * @param key 扩展表key，该值为空，则全查
     * @return 扩展信息，无值返回空map.返回值key是资源id
     */
    List<ProtectedResourceExtendInfo> queryExtendInfo(List<String> resourceIds, String key);

    /**
     * 查询资源的扩展信息
     *
     * @param resourceIds 资源ID集合
     * @return key: 资源ID，value: 扩展信息集合
     */
    Map<String, List<ProtectedResourceExtendInfo>> queryExtendInfoByResourceIds(List<String> resourceIds);
}
