package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * VMWare info
 *
 * @author h30003246
 * @since 2020-12-04
 */
@Data
public class VMWareDetailInfo {
    private String uuid;

    @JsonProperty("hardware")
    private VMWareHardware hardware;

    @JsonProperty("vmx_datastore")
    private VMWareSettingDataStore vmWareSettingDataStore;

    private VMWareRuntime runtime;
}
