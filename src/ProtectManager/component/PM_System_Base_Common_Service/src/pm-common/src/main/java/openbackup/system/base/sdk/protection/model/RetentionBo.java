/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.protection.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

/**
 * 保留时间数据类型
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.0.0]
 * @since 2020/10/9
 **/
@Data
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class RetentionBo {
    /**
     * retention type
     */
    private Integer retentionType;

    /**
     * retention duration
     */
    private Integer retentionDuration;

    /**
     * duration unit
     */
    private String durationUnit;

    /**
     * 副本保留个数
     */
    private Integer retentionQuantity;
}
