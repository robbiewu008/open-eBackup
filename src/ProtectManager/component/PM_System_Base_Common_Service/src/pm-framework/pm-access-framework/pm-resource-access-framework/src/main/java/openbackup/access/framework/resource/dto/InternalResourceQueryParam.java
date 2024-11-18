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
package openbackup.access.framework.resource.dto;

import openbackup.system.base.common.constants.LegoNumberConstant;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;

/**
 * 内部查询接口参数
 *
 */
@Data
public class InternalResourceQueryParam {
    @Min(LegoNumberConstant.ZERO)
    @Max(Integer.MAX_VALUE)
    private int pageNo;

    @Min(LegoNumberConstant.ONE)
    @Max(LegoNumberConstant.VALUE_200)
    private int pageSize = 10;

    private String conditions = "{}";

    @JsonProperty("queryDependency")
    private boolean shouldQueryDependency;

    private String[] orders = new String[]{};
}