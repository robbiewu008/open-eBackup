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
package openbackup.data.access.framework.core.dto;

import lombok.Data;

/**
 * Paging Param Request
 *
 * @author l00272247
 * @since 2020/07/03
 */
@Data
public class PagingParamRequestDto {
    private static final int DEFAULT_PAGE_SIZE = 10;

    private int startPage;

    private int pageSize;

    /**
     * constructor
     */
    public PagingParamRequestDto() {
        this(0, DEFAULT_PAGE_SIZE);
    }

    /**
     * constructor
     *
     * @param index index
     * @param count count
     */
    public PagingParamRequestDto(int index, int count) {
        this.startPage = index;
        this.pageSize = count;
    }
}
