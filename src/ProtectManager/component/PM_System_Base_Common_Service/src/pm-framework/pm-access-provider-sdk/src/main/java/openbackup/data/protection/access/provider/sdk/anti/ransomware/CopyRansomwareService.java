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
package openbackup.data.protection.access.provider.sdk.anti.ransomware;

import java.util.List;
import java.util.Map;

/**
 * 副本勒索服务
 *
 */
public interface CopyRansomwareService {
    /**
     * 检查防勒索感染副本操作是否有效
     *
     * @param copyId 副本ID
     * @param operation 副本操作
     */
    void checkCopyOperationValid(String copyId, String operation);

    /**
     * 检查防勒索感染副本操作是否有效,并返回结果
     *
     * @param copyIds 副本列表
     * @param operation 操作
     * @return 副本操作是否有效，副本是否支持该操作
     */
    Map<String, Boolean> checkCopyOperationValidWithMap(List<String> copyIds, String operation);


    /**
     * 检查副本是否感染,并返回结果
     *
     * @param copyIds 副本列表
     * @return 副本操作是否有效，副本是否支持该操作
     */
    Map<String, Boolean> checkCopyInfectedWithMap(List<String> copyIds);
}
