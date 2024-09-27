/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.core.common.enums;

/**
 * 恢复类型
 *
 * @author p00511147
 * @since 2020-12-29
 */
public enum VmRestoreTypeEnum {
    OVERWRITING("0"),
    SKIP("1"),
    REPLACE("2"),
    DOWNLOAD("3");

    private String mode;

    VmRestoreTypeEnum(String mode) {
        this.mode = mode;
    }

    public String getMode() {
        return this.mode;
    }
}
