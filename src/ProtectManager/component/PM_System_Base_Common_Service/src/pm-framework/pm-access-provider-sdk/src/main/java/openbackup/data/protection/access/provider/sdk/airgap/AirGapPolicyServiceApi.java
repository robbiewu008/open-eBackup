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
package openbackup.data.protection.access.provider.sdk.airgap;

import java.util.List;

/**
 * airGap-policy对外接口
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2022-08-3
 */
public interface AirGapPolicyServiceApi {
    /**
     * query a live mount policy by id
     *
     * @param id id
     * @return boolean 是否存在
     */
    boolean existPolicy(String id);

    /**
     * 策略id列表
     *
     * @return 策略id列表
     */
    List<String> getAirGapPolicyIdList();
}
