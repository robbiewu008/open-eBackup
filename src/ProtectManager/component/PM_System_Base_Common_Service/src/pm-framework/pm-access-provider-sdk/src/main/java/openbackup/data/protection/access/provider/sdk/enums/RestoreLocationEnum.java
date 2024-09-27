/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.enums;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import java.util.Arrays;

/**
 * 恢复目标类型枚举定义
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-19
 */
public enum RestoreLocationEnum {
    /**
     * 新位置恢复
     */
    NEW("new"),
    /**
     * 原位置恢复
     */
    ORIGINAL("original"),
    /**
     * 本机恢复
     */
    NATIVE("native");

    private final String location;

    RestoreLocationEnum(String location) {
        this.location = location;
    }

    @JsonValue
    public String getLocation() {
        return location;
    }

    /**
     * 根据恢复位置获取恢复位置枚举对象
     *
     * @param location 恢复类型
     * @return 恢复任务类型 {@code RestoreLocationEnum}
     */
    @JsonCreator
    public static RestoreLocationEnum getByLocation(String location) {
        return Arrays.stream(RestoreLocationEnum.values())
            .filter(restoreLocation -> restoreLocation.location.equals(location))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
