/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 创建保护资源过滤规则
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-17
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class ResourceFilterDto {
    /**
     * 过滤条件，根据什么字段过滤，如ID、name等
     */
    private String filterColumn;

    /**
     * 过滤的类型：如磁盘、虚拟机等
     */
    private String type;

    /**
     * 过滤规则：如模糊匹配、全匹配等
     */
    private String rule;

    /**
     * 过滤类型：INCLUDE、EXCLUDE
     */
    private String mode;

    /**
     * 具体过滤的值
     */
    private List<String> values;
}
