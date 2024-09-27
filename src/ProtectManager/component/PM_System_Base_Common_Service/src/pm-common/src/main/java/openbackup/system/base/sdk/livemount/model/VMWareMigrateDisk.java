/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.livemount.model;

import lombok.Data;

/**
 * VMWare 迁移请求参数
 *
 * @author h30003246
 * @since 2020-12-31
 */
@Data
public class VMWareMigrateDisk {
    private String diskId;

    private String datastoreMoId;
}
