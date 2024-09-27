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
package openbackup.data.access.framework.protection.mocks;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

/**
 * 资源相关的mock
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/7/22
 */
public class ResourceMocker {
    public static ProtectedResource mockResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource.setUuid("test1");
        return protectedResource;
    }
}
