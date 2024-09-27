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

import java.util.List;

/**
 * 分页模板类
 *
 * @param <T> the body type
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-06-29
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class PageListResponse<T> {
    private int totalCount;

    private int startIndex;

    private int pageSize;

    private int totalPages;

    private List<T> records;

    public PageListResponse(int totalCount, List<T> records) {
        this.totalCount = totalCount;
        this.records = records;
    }
}
