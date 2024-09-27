/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.bean;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 功能描述
 *
 * @author z00842230
 * @since 2024-04-07
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class ConfigMapOperationParam {
    /**
     * 命名空间
     */
    @JsonProperty("nameSpace")
    private String nameSpace;

    /**
     * configMap的名称
     */
    @JsonProperty("configMap")
    private String configMap;

    /**
     * data中的key
     */
    @JsonProperty("configKey")
    private String configKey;

    /**
     * data中的value
     */
    @JsonProperty("configValue")
    private String configValue;
}
