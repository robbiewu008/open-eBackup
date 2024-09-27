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
package openbackup.openstack.protection.access.provider;

import openbackup.openstack.protection.access.dto.CopyVolInfo;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.copy.CopyCommonInterceptor;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.dee.model.CopyCatalogsRequest;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

/**
 * OpenstackCopyCommonInterceptor
 *
 * @author y30037959
 * @since 2024-08-20
 */
@Slf4j
@Component
public class OpenstackCopyCommonInterceptor implements CopyCommonInterceptor {
    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.equalsSubType(resourceSubType);
    }

    @Override
    public void buildCatalogsRequest(Copy copy, CopyCatalogsRequest catalogsRequest) {
        String copyMetaData = CopyVolInfo.convert2IndexDiskInfos(JSONObject.fromObject(copy.getProperties()));
        catalogsRequest.getCopyInfo().setCopyMetaData(copyMetaData);
    }
}
