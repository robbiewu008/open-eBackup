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

/**
 * 查询关联资源信息
 *
 */
public interface ProtectionQueryService {
    /**
     * 根据SLA的ID和userId查询关联资源数量和subType
     *
     * @param slaName SLA的Name
     * @param userId 用户Id
     * @param subType 资源子类型
     * @return 返回数量
     */
    int countBySubTypeAndSlaName(String slaName, String userId, String subType);
}
