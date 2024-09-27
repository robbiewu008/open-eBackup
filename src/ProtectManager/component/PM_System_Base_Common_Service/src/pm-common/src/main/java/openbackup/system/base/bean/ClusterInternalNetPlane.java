/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.bean;

import openbackup.system.base.sdk.cluster.netplane.BondPort;
import openbackup.system.base.sdk.cluster.netplane.PortRoute;
import openbackup.system.base.sdk.cluster.netplane.VlanPortDto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

/**
 * ClusterInternalNetPlane
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-03-06
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class ClusterInternalNetPlane {
    private String ip;

    @JsonProperty("ip_version")
    private String ipType;

    private String mask;

    private String gateway;

    @JsonProperty("home_port_type")
    private String homePortType;

    @JsonProperty("vlan")
    private VlanPortDto vlanPort;

    @JsonProperty("bond_port")
    private BondPort bondPort;

    @JsonProperty("port_list")
    private List<String> portList;

    @JsonProperty("is_share_bond_port")
    private String isShareBondPort;

    @JsonProperty("is_reuse")
    private String isReuse;

    @JsonProperty("port_routes")
    private List<PortRoute> portRoutes;
}
