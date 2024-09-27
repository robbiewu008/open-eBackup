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
package openbackup.data.protection.access.provider.sdk.protection;

import java.util.Map;

/**
 * 操作受保护对形象
 *
 * @author y30044273
 * @version [OceanProtect X8000 1.5.0]
 * @since 2023-08-07
 */
public interface ProtectionOperateService {
    /**
     * 更新受保护对象的扩展字段
     *
     * @param updateMap 修改的扩展字段
     * @param resourceId 资源id
     */
    void updateProtectedObjectExtendParam(Map<String, Object> updateMap, String resourceId);
}
