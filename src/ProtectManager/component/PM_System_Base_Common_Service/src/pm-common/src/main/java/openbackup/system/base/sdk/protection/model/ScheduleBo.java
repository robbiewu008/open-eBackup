/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.protection.model;

import com.fasterxml.jackson.annotation.JsonFormat;
import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import java.util.Date;

/**
 * 调度数据类型
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.0.0]
 * @since  2020/10/9
 **/
@Data
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class ScheduleBo {
    /**
     * trigger type
     */
    private Integer trigger;

    /**
     * trigger interval
     */
    private Integer interval;

    /**
     * trigger interval unit
     */
    private String intervalUnit;

    /**
     * trigger start time
     */
    @JsonFormat(pattern = "yyyy-MM-dd'T'HH:mm:ss")
    private Date startTime;

    /**
     * time window start
     */
    private String windowStart;

    /**
     * time window end
     */
    private String windowEnd;
}
