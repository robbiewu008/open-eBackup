/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.data.protection.access.provider.sdk.resourcegroup.dto;

import com.huawei.oceanprotect.system.base.label.entity.LabelVo;

import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;
import java.util.List;

/**
 * ResourceGroupDto
 *
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

    private List<LabelVo> labelList;

    private String groupType;
}