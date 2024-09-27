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
package openbackup.system.base.common.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;

/**
 * 分页参数
 *
 * @author w00448845
 * @author w00493811
 * @version [BCManager 8.0.0]
 * @since 2019-11-12
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class PagingParamRequest {
    /**
     * 默认起始页
     */
    private static final int DEFAULT_START_PAGE = 0;

    /**
     * 默认页大小
     */
    private static final int DEFAULT_PAGE_SIZE = 10;

    @Min(0)
    private int startPage = DEFAULT_START_PAGE;

    @Min(0)
    @Max(500)
    private int pageSize = DEFAULT_PAGE_SIZE;
}
