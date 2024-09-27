package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * vms datastore信息
 *
 * @author h30003246
 * @since 2020-12-04
 */
@Data
public class VMWareSettingDataStore {
    private String uuid;

    @JsonProperty("mo_id")
    private String moId;

    private String name;
}
