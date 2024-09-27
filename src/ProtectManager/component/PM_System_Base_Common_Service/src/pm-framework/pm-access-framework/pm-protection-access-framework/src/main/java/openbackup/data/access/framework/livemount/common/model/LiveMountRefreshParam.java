/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.livemount.common.model;

import openbackup.system.base.common.model.livemount.LiveMountEntity;

import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

/**
 * Live Mount Refresh Param
 *
 * @author h30003246
 * @since 2021-01-027
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class LiveMountRefreshParam {
    private Boolean hasCleanProtection;

    private LiveMountEntity liveMount;
}
