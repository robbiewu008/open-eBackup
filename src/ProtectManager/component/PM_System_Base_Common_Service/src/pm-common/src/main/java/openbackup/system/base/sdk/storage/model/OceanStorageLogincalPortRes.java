package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * OceanStorageLogincalPortRes
 *
 * @author p00511147
 * @since 2020-2-4
 */
@Data
public class OceanStorageLogincalPortRes {
    @JsonProperty("ADDRESSFAMILY")
    private String addRessfamily;

    @JsonProperty("CANFAILOVER")
    private String canFailover;

    @JsonProperty("CURRENTCONTROLLERID")
    private String currentCountPollerId;

    @JsonProperty("CURRENTPORTID")
    private String currentPortId;

    @JsonProperty("CURRENTPORTNAME")
    private String currentPortName;

    @JsonProperty("CURRENTPORTTYPE")
    private String currentPortType;

    @JsonProperty("FAILBACKMODE")
    private String failBackMode;

    @JsonProperty("FAILOVERGROUPID")
    private String failoverGroupId;

    @JsonProperty("FAILOVERGROUPNAME")
    private String failoverGroupName;

    @JsonProperty("HOMECONTROLLERID")
    private String homeControllerId;

    @JsonProperty("HOMEPORTID")
    private String homePortId;

    @JsonProperty("HOMEPORTNAME")
    private String homePortName;

    @JsonProperty("HOMEPORTTYPE")
    private String homePortType;

    @JsonProperty("ID")
    private String id;

    @JsonProperty("IPV4ADDR")
    private String ipv4Addr;

    @JsonProperty("IPV4GATEWAY")
    private String ipv4Gateway;

    @JsonProperty("IPV4MASK")
    private String ipv4Mask;

    @JsonProperty("IPV6ADDR")
    private String ipv6Addr;

    @JsonProperty("IPV6GATEWAY")
    private String ipv6Gateway;

    @JsonProperty("IPV6MASK")
    private String ipv6Mask;

    @JsonProperty("ISPRIVATE")
    private String isPrivate;

    @JsonProperty("MANAGEMENTACCESS")
    private String managementAccess;

    @JsonProperty("NAME")
    private String name;

    @JsonProperty("OPERATIONALSTATUS")
    private String operationalStatus;

    @JsonProperty("ROLE")
    private String role;

    @JsonProperty("RUNNINGSTATUS")
    private String runingStatus;

    @JsonProperty("SUPPORTPROTOCOL")
    private String supportProtocol;

    @JsonProperty("TYPE")
    private String type;

    @JsonProperty("ddnsStatus")
    private String ddnsStatus;

    @JsonProperty("dnsZoneName")
    private String dnsZoneName;

    @JsonProperty("listenDnsQueryEnabled")
    private String listenDnsQueryEnabled;

    @JsonProperty("isDefLif")
    private boolean isDefLif;

    @JsonProperty("LogicalType")
    private String logicalType;

    @JsonProperty("vstoreId")
    private String vstoreId;
}
