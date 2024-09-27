/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.common.model;

import lombok.Data;

/**
 * CreateUserLogParam
 *
 * @author y30046482
 * @since 2023-09-08
 */
@Data
public class CreateUserLogParam {
    private String platform;
    private String userName;
    private String ip;
}
