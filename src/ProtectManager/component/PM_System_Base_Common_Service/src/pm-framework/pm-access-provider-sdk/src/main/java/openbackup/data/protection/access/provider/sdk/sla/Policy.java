/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.sla;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import java.util.Optional;

/**
 * 保护策略
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-19
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class Policy {
    /**
     * uuid
     */
    private String uuid;

    /**
     * policy type
     */
    private String type;

    /**
     * policy name
     */
    private String name;

    /**
     * backup policy action
     */
    private String action;

    /**
     * policy extend parameters
     */
    private JsonNode extParameters;

    /**
     * policy schedule info
     */
    private Schedule schedule;

    /**
     * policy retention info
     */
    private Retention retention;

    /**
     * 根据key获取policy扩展参数的boolean值
     *
     * @param key 扩展参数的key
     * @return key对应的value
     */
    public Optional<Boolean> getBooleanFormExtParameters(String key) {
        return Optional.ofNullable(extParameters.get(key)).map(JsonNode::asBoolean);
    }
}
