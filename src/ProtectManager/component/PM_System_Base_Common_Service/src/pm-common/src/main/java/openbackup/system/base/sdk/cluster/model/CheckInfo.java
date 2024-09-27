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
package openbackup.system.base.sdk.cluster.model;

import lombok.Getter;
import lombok.NonNull;
import lombok.Setter;

/**
 * 功能说明 检查满足度结果
 *
 * @author x30046484
 * @since 2023-05-11
 */

@Getter
@Setter
public class CheckInfo {
    /**
     * 检查结果
     */
    @NonNull
    private Integer checkResult;

    /**
     * 失败类型
     */
    private Integer failureType;
}
