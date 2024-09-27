/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.access.framework.resource.validator.enums;

/**
 * JsonSchema中需获取的字段的关键字枚举
 *
 * @author w00616953
 * @since 2021-10-19
 */
public enum SchemaKeywordsEnum {
    SECRET("secret"), // jsonschema文件中，关键字为secret的元素名
    EDITABLE("editable"); // jsonschema文件中，关键字为editable的元素名

    private String value;

    SchemaKeywordsEnum(String value) {
        this.value = value;
    }

    public String getValue() {
        return value;
    }
}
