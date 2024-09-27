/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.livemount.common.model;

import openbackup.system.base.common.model.livemount.LiveMountEntity;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import java.util.Map;

/**
 * Live Mount Migrate Param
 *
 * @author h30003246
 * @since 2021-01-05
 */
@Data
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class LiveMountMigrateParam {
    private String requestId;

    private String jobId;

    private LiveMountEntity liveMount;

    private Map<String, Object> liveMountMigrateExtParam;
}
