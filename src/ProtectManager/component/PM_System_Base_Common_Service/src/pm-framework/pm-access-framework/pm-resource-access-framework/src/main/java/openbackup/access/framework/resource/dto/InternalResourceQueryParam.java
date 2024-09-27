/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * @author mwx1009754
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-06
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