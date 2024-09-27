/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.accesspoint.model.enums;

import lombok.ToString;

/**
 * 初始化备份错误码
 *
 * @author swx1010572
 * @since 2021-01-15
 */
@ToString
public enum InitNetworkResultCode {
    /**
     * 成功
     */
    SUCCESS(0),

    /**
     * 失败
     */
    FAILURE(-1);

    /**
     * 编码
     */
    private final int code;

    /**
     * 带参数初始化函数
     *
     * @param theCode 编码
     */
    InitNetworkResultCode(int theCode) {
        code = theCode;
    }

    /**
     * 是否OK
     *
     * @return 是否OK
     */
    public boolean isOkay() {
        return this == SUCCESS;
    }
}
