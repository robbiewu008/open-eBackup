/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.sftp.model;

import lombok.Data;

/**
 * sftp response
 *
 * @author dWX1009286
 * @since 2021-06-10
 */
@Data
public class SftpResponse {
    private boolean isSuccess;

    private String code;

    private String message;
}
