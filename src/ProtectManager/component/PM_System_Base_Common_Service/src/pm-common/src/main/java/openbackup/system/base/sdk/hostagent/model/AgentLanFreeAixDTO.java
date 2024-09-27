/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.hostagent.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

import javax.validation.Valid;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

/**
 * AIX主机 LAN-FREE 配置页信息
 *
 * @author hwx1164326
 * @since 2023-08-22
 */
@Getter
@Setter
public class AgentLanFreeAixDTO {
    @NotNull
    private boolean isDelete;

    /**
     * 选择的数据协议 FC/ISCSI
     */
    @Pattern(regexp = "FC|ISCSI")
    private String dataProtocol = "FC";

    /**
     * 选择的SanClient主机id
     */
    @NotNull
    private List<String> sanclientResourceIds;

    /**
     * AIX主机 WWPN信息
     */
    @Valid
    private List<AgentLanFreeWwpn> clientWwpns;

    /**
     * AIX主机 IQN信息
     */
    private List<String> clientIqns;
}