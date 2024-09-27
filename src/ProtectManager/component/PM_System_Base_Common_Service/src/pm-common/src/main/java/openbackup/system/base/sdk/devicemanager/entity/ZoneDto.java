package openbackup.system.base.sdk.devicemanager.entity;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * pacific access zone
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-03
 */
@Getter
@Setter
public class ZoneDto {
    // dns策略  1：轮询方式
    @JsonProperty("dns_strategy")
    private Integer dnsStrategy;

    // 子域名
    @JsonProperty("domain")
    private String domain;

    // 是否启用DNS服务
    @JsonProperty("enable_dns")
    private boolean isEnableDns;

    // 是否启用ip故障漂移
    @JsonProperty("enable_ip_drift")
    private boolean isEnableIpDrift;

    // 网络类型：以太
    @JsonProperty("network_type")
    private Integer networkType;

    // access zone名称
    @JsonProperty("name")
    private String name;

    // 子网名称
    @JsonProperty("subnet_name")
    private String subnetName;

    // access zone 类型
    @JsonProperty("zone_type")
    private Integer zoneType;

    // 账号id
    @JsonProperty("account_id")
    private Integer accountId;

    // 账号名称
    @JsonProperty("account_name")
    private String accountName;
}
