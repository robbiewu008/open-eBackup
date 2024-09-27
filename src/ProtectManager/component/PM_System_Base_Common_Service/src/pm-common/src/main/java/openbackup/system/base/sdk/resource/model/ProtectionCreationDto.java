/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import java.util.List;
import java.util.Map;

/**
 * 保护创建请求体DTO
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-17
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class ProtectionCreationDto {
    /**
     * 绑定SLA的id
     */
    private String slaId;

    /**
     * 保护的资源列表
     */
    private List<ProtectionResourceDto> resources;

    /**
     * 扩展属性
     */
    private Map<String, Object> extParameters;

    /**
     * 绑定SLA成功后执行的操作
     */
    private String postAction;
}
