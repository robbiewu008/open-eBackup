/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.sla;

import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

/**
 * 保留策略
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-19
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class Retention {
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
}
