/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.model.storage;

import lombok.Data;

import javax.validation.constraints.NotNull;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-06-29
 */
@Data
public class UpdateArchiveStorage {
    private Integer connectType;

    private String userName;

    private String password;

    private String storageId;

    private String certId;

    private boolean alarmEnable;

    private long alarmThreshold;

    @NotNull
    private String alarmLimitValueUnit;
}
