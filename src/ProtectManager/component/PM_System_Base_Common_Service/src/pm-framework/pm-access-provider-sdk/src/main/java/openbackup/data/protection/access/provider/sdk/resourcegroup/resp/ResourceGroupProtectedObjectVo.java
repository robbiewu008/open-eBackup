/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resourcegroup.resp;

import openbackup.system.base.common.utils.JSONObject;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

/**
 * 资源组保护对象
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-25
 */

@Getter
@Setter
public class ResourceGroupProtectedObjectVo {
    private String uuid;

    private String slaId;

    private String slaName;

    @JsonProperty("slaCompliance")
    private Boolean isSlaCompliance;

    private int status;

    private String name;

    private String envId;

    private String envType;

    private String resourceId;

    private String resourceGroupId;

    private JSONObject extParameters;

    private String type;

    private String subType;

    private String path;

    private Timestamp latestTime;

    private Timestamp earliestTime;

    private String chainId;

    private String consistentStatus;

    private String consistentResults;
}