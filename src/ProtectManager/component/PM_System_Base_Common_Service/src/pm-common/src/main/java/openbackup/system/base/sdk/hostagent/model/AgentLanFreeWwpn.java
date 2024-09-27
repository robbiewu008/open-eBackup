/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.hostagent.model;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Pattern;

/**
 * LAN-FREE配置页面WWPN信息
 *
 * @author hwx1164326
 * @since 2023-08-22
 */
@Getter
@Setter
public class AgentLanFreeWwpn {
    /**
     * 客户端wwpn
     */
    @Pattern(regexp = "^[0-9A-Fa-f]{16}$")
    private String wwpn;

    /**
     * 是否客户页面添加的
     */
    private boolean isManualAdd;

    /**
     * 是否选择
     */
    private boolean isChosen;

    /**
     * 运行状态
     */
    @Length(min = 1, max = 64)
    private String runningStatus;
}
