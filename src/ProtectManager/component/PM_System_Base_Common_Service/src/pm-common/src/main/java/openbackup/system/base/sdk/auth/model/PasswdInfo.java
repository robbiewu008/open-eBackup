/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

/**
 * PasswdInfo
 *
 * @author y30021475
 * @since 2023-09-13
 */
@Getter
@Setter
public class PasswdInfo {
    private String id;

    private String newPasswd;

    private String component;

    private String subComponent;

    private String accountName;
}
