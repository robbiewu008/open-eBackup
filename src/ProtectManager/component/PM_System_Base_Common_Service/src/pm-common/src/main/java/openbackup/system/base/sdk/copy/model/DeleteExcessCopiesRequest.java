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
package openbackup.system.base.sdk.copy.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 删除资源多余副本请求体
 *
 */
@Getter
@Setter
public class DeleteExcessCopiesRequest {
    /**
     * 副本保留数量
     */
    private int retentionQuantity;

    /**
     * 副本生成方式
     */
    private String generatedBy;

    /**
     * 用户id
     */
    private String userId;
}
