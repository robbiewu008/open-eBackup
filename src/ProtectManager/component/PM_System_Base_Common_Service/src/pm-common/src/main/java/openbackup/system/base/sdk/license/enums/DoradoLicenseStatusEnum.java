/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.license.enums;

import lombok.Getter;

/**
 * 底座license状态
 *
 * @author g00500588
 * @since 2022/06/06
 */
@Getter
public enum DoradoLicenseStatusEnum {
    VALID(1, "有效"),
    EXPIRED(2, "过期"),
    INVALID(3, "无效");

    DoradoLicenseStatusEnum(int status, String desc) {
        this.status = status;
        this.desc = desc;
    }

    private final int status;

    private final String desc;
}
