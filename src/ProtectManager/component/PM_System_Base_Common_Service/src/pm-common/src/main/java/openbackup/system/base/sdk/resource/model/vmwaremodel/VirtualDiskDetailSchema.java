package openbackup.system.base.sdk.resource.model.vmwaremodel;

import openbackup.system.base.sdk.resource.model.DataStore;
import openbackup.system.base.sdk.resource.model.ResourceEntity;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 虚拟机disk信息
 *
 * @author t00482481
 * @since 2020-11-04
 */
@Data
public class VirtualDiskDetailSchema extends ResourceEntity {
    private String slot;

    private Long capacity;

    @JsonProperty("datastore")
    private DataStore dataStore;

    @JsonProperty("disk_lun")
    private String diskLun;

    @JsonProperty("disk_type")
    private String diskType;

    @JsonProperty("is_nas")
    private boolean isNas;

    @JsonProperty("nas_info")
    private HostNasVolume nasInfo;
}
