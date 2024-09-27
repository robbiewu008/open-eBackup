package openbackup.system.base.sdk.accesspoint.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * Dme Remote Device
 *
 * @author l00272247
 * @since 2020-12-16
 */
@Data
public class DmeRemoteDevice {
    private int port;

    @JsonProperty("portPM")
    private int portPm;

    @JsonProperty("userNamePM")
    private String userNamePm;

    @JsonProperty("passwordPM")
    private String passwordPm;

    @JsonProperty("tokenPM")
    private String tokenPM;

    private String cert;

    @JsonProperty("ESN")
    private String esn;

    @JsonProperty("mgrIp")
    private List<String> mgrIpList;

    @JsonProperty("netplaneinfo")
    private String netPlaneInfo;

    private String networkInfo;

    private String storageId;

    private String deployType;

    private String poolId;

    // op和存储不在一台机器上时，标识op的esn
    private String backupSoftwareEsn;

    // 远端存储类型
    private String remoteStorageType;
}
