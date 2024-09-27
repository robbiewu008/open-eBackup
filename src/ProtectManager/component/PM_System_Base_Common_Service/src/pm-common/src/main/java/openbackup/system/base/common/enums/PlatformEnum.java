/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.common.enums;

import lombok.Getter;

/**
 * 平台枚举类
 *
 * @author y30046482
 * @since 2023-09-08
 */
@Getter
public enum PlatformEnum {
    OCEAN_PROTECT("OceanProtect"),
    CYBER_ENGINE("CyberEngine"),
    HUAWEI_CLOUD_STACK("HuaweiCloudStack");
    private final String name;

    PlatformEnum(String name) {
        this.name = name;
    }
}
