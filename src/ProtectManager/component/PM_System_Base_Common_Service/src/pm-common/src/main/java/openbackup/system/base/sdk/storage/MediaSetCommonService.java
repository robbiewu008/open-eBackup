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

import openbackup.system.base.common.model.repository.tape.TapeSetDetailResponse;

/**
 * 介质集管理业务层
 *
 */
public interface MediaSetCommonService {
    /**
     * 获取介质集详情
     *
     * @param mediaSetId 介质集UUID
     * @return 介质集详情
     */
    TapeSetDetailResponse getTapeSetDetail(String mediaSetId);
}
