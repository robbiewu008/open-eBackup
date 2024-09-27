/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.systembackup.model;

import lombok.Data;

/**
 * system backup response
 *
 * @author t00508428
 * @since 2020-12-10
 */
@Data
public class SystemBackupResponse {
    private boolean success;

    private String code;

    private String message;
}
