/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

/**
 * 同步组件ip请求体
 *
 * @author x30046484
 * @since 2023-05-18
 */
@Getter
@Setter
public class SyncComponentIpRequest {
    @NotNull
    private List<@Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "Ip is invalid.") String> ipList;

    @NotNull
    private String syncMode;

    @NotNull
    private boolean shouldRestart = false;
}
