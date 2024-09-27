/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.openstack.adapter.enums;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import java.util.stream.Stream;

/**
 * 云核OpenStack备份对象
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-05
 */
public enum OpenStackJobType {
    /**
     * 虚拟机备份
     */
    SERVER("server"),

    /**
     * 卷备份
     */
    VOLUME("volume");

    private final String type;

    OpenStackJobType(String type) {
        this.type = type;
    }

    @JsonValue
    public String getType() {
        return type;
    }

    /**
     * 通过type获取BackupJobType枚举实例
     *
     * @param type type
     * @return OpenStackJobType
     */
    @JsonCreator
    public static OpenStackJobType create(String type) {
        return Stream.of(OpenStackJobType.values())
                .filter(openStackJobType -> openStackJobType.type.equals(type))
                .findFirst()
                .orElseThrow(() -> new IllegalArgumentException("Backup type is illegal."));
    }
}
