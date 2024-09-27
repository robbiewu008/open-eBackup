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

import lombok.Data;

import org.hibernate.validator.constraints.Length;
import org.springframework.validation.annotation.Validated;

/**
 * Sorting parameter object
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2019-12-23
 */
@Data
@Validated
public class SortingParamRequest {
    /**
     * 升序
     */
    public static final String ASC = "asc";

    /**
     * 降序
     */
    public static final String DES = "desc";

    /**
     * 开始时间时间
     */
    public static final String START_TIME = "START_TIME";

    @Length(max = 256)
    private String orderBy;

    @Length(max = 64, message = "orderType must be asc or desc")
    private String orderType;
}
