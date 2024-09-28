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
package openbackup.system.base.sdk.user;

/**
 * 功能描述
 *
 */
public interface DomainResourceSetServiceApi {
    /**
     * 判断是否存在域-资源集关联关系
     *
     * @param domainId 域id
     * @param resourceObjectId 资源id
     * @param type 资源类型
     * @return 是否存在
     */
    String getResourceSetType(String domainId, String resourceObjectId, String type);
}
