package openbackup.data.access.framework.core.common.model;

import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * Global Search index
 *
 * @author l00347293
 * @since 2021-01-12
 **/
@Getter
@Setter
public class ScanRequest extends SearchBaseMsg {
    @JsonProperty("snap_info")
    private SnapInfo snapInfo;

    @JsonProperty("storage_info")
    private RestoreStorageInfo storageInfo;

    @JsonProperty("storage_repository")
    private StorageRepository storageRepository;
}
