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
package openbackup.data.protection.access.provider.sdk.livemount;

import java.util.List;

/**
 * 及时挂载policy对外接口
 *
 */
public interface PolicyServiceApi {
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
    List<String> getPolicyIdList();

    /**
     * 查询挂载更新策略的所有数据
     *
     * @return 挂载更新策略的所有数据
     */
    Integer getAllCount();
}
