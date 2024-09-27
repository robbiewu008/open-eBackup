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
package openbackup.gaussdbdws.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * DWS provider 下发参数工具类
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-05
 */
public class MockProviderParameter {
    public static ProtectedResource getProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setRootUuid("xxxxxx");
        protectedResource.setUuid("aaaa");
        protectedResource.setName("schema");
        protectedResource.setExtendInfo(new HashMap<>());
        protectedResource.getExtendInfo().put(DwsConstant.EXTEND_INFO_KEY_TABLE,"/table,/tableName");
        return protectedResource;
    }

    public static PageListResponse<ProtectedResource> getProtectedResourcePageListResponse() {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setTotalCount(2);
        List<ProtectedResource> protectedResources = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        protectedEnvironment.setRootUuid("xxxxxx");
        protectedEnvironment.setExtendInfo(new HashMap<>());
        protectedEnvironment.getExtendInfo().put(DwsConstant.EXTEND_INFO_KEY_TABLE,"/table1,/tableName1,/database1/schema1");
        protectedEnvironment.setUuid("bbbb");
        protectedEnvironment.setName("/database1/schema1");
        protectedResources.add(protectedEnvironment);
        ProtectedEnvironment protectedEnvironment2 = new ProtectedEnvironment();
        protectedEnvironment2.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        protectedEnvironment2.setRootUuid("xxxxxx");
        protectedEnvironment2.setExtendInfo(new HashMap<>());
        protectedEnvironment2.getExtendInfo().put(DwsConstant.EXTEND_INFO_KEY_TABLE,"/table1,/tableName2");
        protectedEnvironment2.setUuid("bbbb");
        protectedEnvironment2.setName("/database1/schema3");
        protectedResources.add(protectedEnvironment2);
        pageListResponse.setRecords(protectedResources);
        return pageListResponse;
    }
}
