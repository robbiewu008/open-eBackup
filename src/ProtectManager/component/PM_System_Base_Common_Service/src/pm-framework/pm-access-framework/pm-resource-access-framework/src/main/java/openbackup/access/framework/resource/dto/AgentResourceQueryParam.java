/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
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