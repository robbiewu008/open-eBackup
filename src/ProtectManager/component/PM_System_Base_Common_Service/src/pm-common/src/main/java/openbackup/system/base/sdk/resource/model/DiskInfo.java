/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * vmware副本中的磁盘信息
 *
 * @author tantong
 * @since 2020-11-29
 */
@Getter
@Setter
public class DiskInfo {
    @JsonProperty("DSNAME")
    private String dsName;

    @JsonProperty("DSMOREF")
    private String dsMoRef;

    @JsonProperty("DSSIZE")
    private String dsSize;

    @JsonProperty("BUSNUMBER")
    private String busNumber;

    @JsonProperty("CONTROLLERTYPE")
    private String controllerType;

    @JsonProperty("GUID")
    private String guId;

    @JsonProperty("DISKTYPE")
    private String diskType;

    @JsonProperty("NAME")
    private String name;

    @JsonProperty("SIZE")
    private String size;

    @JsonProperty("DISKUNIQUEPATH")
    private String diskUniquePath;

    @JsonProperty("DISKDEVICENAME")
    private String diskDeviceName;

    @JsonProperty("DISKSNAPSHOTDEVICENAME")
    private String diskSnapShotDeviceName;

    @JsonProperty("DSIP")
    private String dsIp;
}
