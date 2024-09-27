/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
