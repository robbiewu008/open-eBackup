/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import java.util.Map;

/**
 * Protected Object Info
 *
 * @author l00272247
 * @since 2020-01-11
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class ProtectedObjectInfo {
    private String slaId;
    private String slaName;
    private String resourceId;
    private String name;
    private String path;
    private String envId;
    private String envType;
    private String type;
    private String subType;
    private int status;
    private Map<String, Object> extParameters;
}
