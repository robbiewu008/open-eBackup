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
package openbackup.db2.protection.access.provider.resource;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBrowseProvider;
import openbackup.db2.protection.access.service.Db2TablespaceService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

/**
 * db2主机上表空间浏览
 *
 */
@Component
@Slf4j
public class Db2HostTablespaceBrowseProvider implements ResourceBrowseProvider {
    private final Db2TablespaceService db2TablespaceService;

    public Db2HostTablespaceBrowseProvider(Db2TablespaceService db2TablespaceService) {
        this.db2TablespaceService = db2TablespaceService;
    }

    @Override
    public PageListResponse<ProtectedResource> browse(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        log.info("Start browse db2 single tablespace. database id: {}", environmentConditions.getParentId());
        PageListResponse<ProtectedResource> detailPageList = db2TablespaceService.querySingleTablespace(environment,
            environmentConditions);
        db2TablespaceService.setTablespaceLockedStatus(environmentConditions.getParentId(), detailPageList);
        log.info("End browse db2 single tablespace. database id: {}, size: {}", environmentConditions.getParentId(),
            detailPageList.getRecords().size());
        return detailPageList;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.DB2_TABLESPACE.equalsSubType(subType);
    }
}
