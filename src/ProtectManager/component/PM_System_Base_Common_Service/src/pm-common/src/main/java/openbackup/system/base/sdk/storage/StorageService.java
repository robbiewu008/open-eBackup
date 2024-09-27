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
package openbackup.system.base.sdk.storage;

import openbackup.system.base.common.model.repository.StoragePool;
import openbackup.system.base.common.model.storage.StorageResponse;
import openbackup.system.base.common.model.storage.StorageSession;

import java.util.List;

/**
 * 存储服务
 *
 * @author y00413474
 * @author w00493811
 * @since 2020-07-02
 */
public interface StorageService {
    /**
     * 查询存储信息
     *
     * @return 存储信息
     */
    StorageResponse<StorageSession> getStorageSession();

    /**
     * 查询存储池
     *
     * @return 存储池
     */
    StorageResponse<List<StoragePool>> getStoragePools();

    /**
     * 设置存储池告警阈值
     *
     * @param storagePoolId 存储池编号
     * @param userConsumedCapacityThreshold 容量告警阈值(%)
     * @return 操作结果
     */
    StorageResponse<Object> setStoragePoolAlarmThreshold(String storagePoolId, int userConsumedCapacityThreshold);
}
