/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.system.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * DM时区信息
 *
 * @author z00445440
 * @since 2023-03-03
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class TimeZoneInfo {
    @JsonProperty("CMO_SYS_TIME_ZONE")
    private String cmoSysTimeZone;

    @JsonProperty("CMO_SYS_TIME_ZONE_NAME")
    private String cmoSysTimeZoneName;

    @JsonProperty("CMO_SYS_TIME_ZONE_NAME_STYLE")
    private String cmoSysTimeZoneNameStyle;

    @JsonProperty("CMO_SYS_TIME_ZONE_USE_DST")
    private String cmoSysTimeZoneUseDst;
}
