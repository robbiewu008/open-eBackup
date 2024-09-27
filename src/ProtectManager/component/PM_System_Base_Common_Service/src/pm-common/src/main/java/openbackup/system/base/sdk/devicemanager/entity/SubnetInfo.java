package openbackup.system.base.sdk.devicemanager.entity;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * pacific 子网信息
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-19
 */
@Getter
@Setter
public class SubnetInfo {
    @JsonProperty("dns_iface_name")
    private String dnsIfaceName;

    @JsonProperty("dns_ip")
    private String dnsIp;

    private String domain;

    private String gateway;

    private String name;

    @JsonProperty("net_version")
    private String netVersion;

    @JsonProperty("node_storage_frontend_ip")
    private String nodeStorageFrontendIp;

    @JsonProperty("standby_dns_ip")
    private String standbyDnsIp;

    @JsonProperty("vlan_id")
    private int vlanId;
}
