/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.request;

import lombok.Data;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-07-22
 */
@Data
public class RecoveryTaskRequest {
    @NotNull
    private Long imageId;

    @NotNull
    @Size(max = 256)
    private String password;

    @NotNull
    @Size(max = 1024)
    private String remoteAddress;
}
