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
package openbackup.system.base.sdk.anti.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import org.hibernate.validator.constraints.Length;

import java.util.List;

import javax.validation.constraints.NotNull;

/**
 * 批量查询防勒索策略请求
 *
 * @author j00619968
 * @since 2023-09-05
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class BatchQueryAntiRansomwarePolicyReq {
    /**
     * 资源ID列表
     */
    @NotNull
    List<@Length(min = 1, max = 64) String> resourceIds;
}
