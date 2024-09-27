/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;
import java.util.Map;

/**
 * Resource entity, this class entity Indicates a specific resource, for example db, file system, vm.
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-17
 */
@Data
public class Resource {
    /**
     * FILE_SYSTEM
     */
    public static final String FILE_SYSTEM = "Fileset";

    /**
     * Oracle
     */
    public static final String ORACLE = "Oracle";

    /**
     * VMware
     */
    public static final String VMWARE = "VMware";

    private String uuid;

    private String name;

    private String type;

    @JsonProperty("sub_type")
    private String subType;

    private String path;

    @JsonProperty("parent_uuid")
    private String parentUuid;

    @JsonProperty("root_uuid")
    private String rootUuid;

    private boolean hasChildren;

    private String size;

    @JsonProperty("environment_name")
    private String environmentName;

    @JsonProperty("environment_endpoint")
    private String environmentEndpoint;

    @JsonProperty("instance_names")
    private String instanceNames;

    @JsonIgnore
    private List<Resource> children;

    // 爱数副本的gnsPath
    @JsonProperty("gns_path")
    private String gnsPath;

    private Map<String, String> extendInfo;
}
