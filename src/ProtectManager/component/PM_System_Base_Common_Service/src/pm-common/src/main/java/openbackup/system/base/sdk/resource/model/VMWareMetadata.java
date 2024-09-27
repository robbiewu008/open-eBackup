/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * VMWare metadata
 *
 * @author h30003246
 * @since 2020-12-04
 */
@Data
public class VMWareMetadata {
    @JsonProperty("disk_info")
    List<DiskInfo> diskInfoList;

    @JsonProperty("net_work")
    List<String> networkNameList;

    @JsonProperty("hardware")
    VMWareHardware vmWareHardware;

    @JsonProperty("vmx_datastore")
    VMWareSettingDataStore vmWareSettingDataStore;

    @JsonProperty("runtime")
    VMWareRuntime vmWareRuntime;
}
