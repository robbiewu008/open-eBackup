/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.common.constants.IsmNumberConstant;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 组件信息
 *
 * @author x30046484
 * @since 2023-05-17
 */
@Getter
@Setter
public class ComponentIpInfo {
    @NotNull
    @Size(min = 1, max = 256)
    private String componentName;

    @Size(max = IsmNumberConstant.TWO_HUNDRED_FIFTY_SIX, min = IsmNumberConstant.ONE, message = "The length of ip is 1-256 characters")
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "cluster ip is invalid, not ipv4 or ipv6.")
    private String ip;
}
