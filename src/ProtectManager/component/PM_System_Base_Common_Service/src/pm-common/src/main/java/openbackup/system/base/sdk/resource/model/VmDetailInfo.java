package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * VM hardware信息
 *
 * @author h30003246
 * @since 2020-12-04
 */
@Data
public class VmDetailInfo {
    private String uuid;

    @JsonProperty("hardware")
    private VmHardware hardware;

    @JsonProperty("vmx_datastore")
    private VmSettingDataStore vmSettingDataStore;
}
