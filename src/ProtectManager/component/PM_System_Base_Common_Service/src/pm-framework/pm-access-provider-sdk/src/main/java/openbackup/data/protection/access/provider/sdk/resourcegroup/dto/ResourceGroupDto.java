/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resourcegroup.dto;

import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;
import java.util.List;

/**
 * ResourceGroupDto
 *
 * @author c00631681
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-26
 */
@Getter
@Setter
public class ResourceGroupDto {
    private String uuid;

    private String name;

    private String path;

    private String sourceType;

    private String sourceSubType;

    private Timestamp createdTime;

    private String extendStr;

    private String userId;

    private ResourceGroupProtectedObjectDto protectedObjectDto;

    private List<ResourceGroupMemberDto> resources;

    private Integer protectionStatus;

    private String scopeResourceId;
}