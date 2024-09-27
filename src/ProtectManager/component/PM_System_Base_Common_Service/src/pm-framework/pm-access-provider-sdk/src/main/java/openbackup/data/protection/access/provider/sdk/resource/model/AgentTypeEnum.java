/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 */

package openbackup.data.protection.access.provider.sdk.resource.model;

/**
 * agent类型枚举类
 *
 * @author w00616953
 * @since 2022-03-22
 */
public enum AgentTypeEnum {
    /**
     * 外置代理
     */
    EXTERNAL_AGENT("0"),

    /**
     * 内置代理
     */
    INTERNAL_AGENT("1");

    private final String value;

    AgentTypeEnum(String value) {
        this.value = value;
    }

    /**
     * 获取枚举值
     *
     * @return 枚举值
     */
    public String getValue() {
        return value;
    }
}
