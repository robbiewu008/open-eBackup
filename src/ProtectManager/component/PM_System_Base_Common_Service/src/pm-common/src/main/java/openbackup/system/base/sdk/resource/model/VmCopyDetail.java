package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * VM hardware信息
 *
 * @author h30003246
 * @since 2020-12-04
 */
@Data
public class VmCopyDetail {
    @JsonProperty("disk_info")
    List<DiskInfo> diskInfoList;

    @JsonProperty("net_work")
    List<String> networkNameList;

    @JsonProperty("hardware")
    VmHardware vmHardware;

    @JsonProperty("setting_data_store")
    VmSettingDataStore vmSettingDataStore;
}
