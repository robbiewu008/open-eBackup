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

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import org.hibernate.validator.constraints.Length;

import java.util.Collections;
import java.util.List;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;

/**
 * 功能描述: AgentResourceQueryParam
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-01
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class AgentResourceQueryParam {
    @NotBlank
    @Length(min = LegoNumberConstant.ONE, max = LegoNumberConstant.TWO_HUNDRED_FIFTY_FIVE)
    private String envId;

    @Min(LegoNumberConstant.ONE)
    @Max(Integer.MAX_VALUE)
    private int pageNo;

    @Min(LegoNumberConstant.ONE)
    @Max(LegoNumberConstant.THOUSAND)
    private int pageSize;

    @Length(max = LegoNumberConstant.TWO_HUNDRED_FIFTY_FIVE)
    private String orders = "";

    @Length(max = LegoNumberConstant.THROUND_TWENTY_FOUR)
    private String appType = "";

    @Length(max = LegoNumberConstant.THROUND_TWENTY_FOUR)
    private String conditions = "";

    private List<String> resourceIds = Collections.emptyList();
}