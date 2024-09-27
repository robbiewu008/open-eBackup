/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.lock;

import com.fasterxml.jackson.annotation.JsonValue;

import java.util.Arrays;
import java.util.Objects;

/**
 * 资源锁类型
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/1/26
 **/
public enum LockTypeEnum {
    READ("r"),
    WRITE("w");

    private final String type;

    LockTypeEnum(String type) {
        this.type = type;
    }

    /**
     * 获取资源锁类型
     *
     * @return 资源锁类型
     */
    @JsonValue
    public String getType() {
        return type;
    }

    /**
     * 根据锁类型获取锁类型枚举
     *
     * @param type 锁类型
     * @return LockTypeEnum
     */
    public static LockTypeEnum getByType(String type) {
        return Arrays.stream(LockTypeEnum.values())
            .filter(item -> Objects.equals(item.type, type))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
