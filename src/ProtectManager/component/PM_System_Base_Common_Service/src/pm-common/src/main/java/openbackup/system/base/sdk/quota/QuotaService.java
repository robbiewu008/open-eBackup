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
package openbackup.system.base.sdk.quota;

import openbackup.system.base.sdk.copy.model.Copy;

import java.util.List;

/**
 * 删除用户的配额信息
 *
 */
public interface QuotaService {
    /**
     * 根据userId,resourceId删除额度记录
     *
     * @param userId 用户id
     * @param resourceId 资源id
     */
    void deleteUserQuotaByUserIdAndResourceId(String userId, String resourceId);

    /**
     * 根据userId初始化hcs用户额度记录
     *
     * @param userId 用户id
     */
    void initHcsUserQuota(String userId);

    /**
     * 校验当前用户对所提供的副本List是否有足够的配额
     *
     * @param userId 用户id
     * @param copyList 副本列表
     * @param quotaTaskTypeEnum 配额类型
     * @return 配额是否充足
     */
    boolean checkIsUserHasEnoughQuota(String userId, List<Copy> copyList, String quotaTaskTypeEnum);

    /**
     * 增加用户已使用配额
     *
     * @param userId 用户id
     * @param resourceId 资源Id
     * @param copyGeneratedBy 副本生成方式
     * @param updateType 增加或减少枚举类
     * @param dataBeforeReduction 缩减前数据量
     */
    void updateUserUsedQuota(String userId, String resourceId, String copyGeneratedBy, String updateType,
        String dataBeforeReduction);
}
