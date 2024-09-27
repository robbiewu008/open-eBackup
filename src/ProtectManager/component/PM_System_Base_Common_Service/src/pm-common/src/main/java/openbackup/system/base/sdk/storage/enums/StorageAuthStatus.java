/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.storage.enums;

/**
 * dorado设备信息
 *
 * @author p00511147
 * @since 2020-11-16
 */
public enum StorageAuthStatus {
    /**
     * 正常
     */
    NORMAL("Normal"),

    /**
     * 异常
     */
    ABNORMAL("Abnormal"),

    /**
     * 未认证
     */
    UNAUTHENTICATED("Unauthenticated");

    private final String type;

    StorageAuthStatus(String type) {
        this.type = type;
    }

    public String getType() {
        return type;
    }
}
