/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.netplane;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

import javax.validation.constraints.NotBlank;
import javax.validation.constraints.Pattern;

/**
 * VlanPort
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-04-07
 */
@Getter
@Setter
public class VlanPortVo {
    @NotBlank
    // 1:以太网端口 2:绑定端口
    @Pattern(regexp = "[17]", message = "value invalid")
    String portType;

    List<@Pattern(regexp = "[0-9]*", message = "value invalid") String> tags;
}
