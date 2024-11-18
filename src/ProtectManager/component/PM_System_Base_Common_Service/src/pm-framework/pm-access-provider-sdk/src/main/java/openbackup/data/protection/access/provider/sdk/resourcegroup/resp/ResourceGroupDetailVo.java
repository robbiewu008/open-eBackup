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
package openbackup.data.protection.access.provider.sdk.resourcegroup.resp;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 资源组详情查询返回体
 *
 */

@Getter
@Setter
public class ResourceGroupDetailVo {
    private String uuid;

    private String name;

    private String path;

    private String sourceType;

    private String sourceSubType;

    private String createdTime;

    private int protectionStatus;

    private String userId;

    private ResourceGroupProtectedObjectVo protectedObject;

    private List<ResourceGroupMemberVo> resourceGroupMembers;
}