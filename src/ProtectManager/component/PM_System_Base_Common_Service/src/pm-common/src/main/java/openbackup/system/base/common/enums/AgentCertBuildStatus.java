/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 */

package openbackup.system.base.common.enums;

import lombok.Getter;

/**
 * 组装Agent证书
 *
 * @author h00406347
 * @since 2021-01-20
 */
public enum AgentCertBuildStatus {
    SUCCESS(0),
    FAIL(1),
    REPLACED(2);

    @Getter
    private final int status;

    AgentCertBuildStatus(int status) {
        this.status = status;
    }
}
