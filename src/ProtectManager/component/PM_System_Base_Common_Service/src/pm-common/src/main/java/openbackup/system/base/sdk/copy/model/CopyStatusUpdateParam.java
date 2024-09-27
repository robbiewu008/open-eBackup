/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import java.util.Date;

/**
 * Copy Status Update Param
 *
 * @author l00272247
 * @since 2020-10-30
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
@JsonInclude(JsonInclude.Include.NON_DEFAULT)
public class CopyStatusUpdateParam {
    private CopyStatus status;

    @JsonProperty("deletable")
    private Boolean isDeletable;

    private String timestamp;

    private String displayTimestamp;

    private Boolean isReplicated;

    private String generatedTime;

    private Date expirationTime;
}
