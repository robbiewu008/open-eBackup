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

import com.google.common.collect.ImmutableList;

import openbackup.data.protection.access.provider.sdk.repository.StorageUnitGroupCheckProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

/**
 * GaussDBDWSStorageUnitGroupCheckProvider
 *
 */
@Component
public class GaussDBDWSStorageUnitGroupCheckProvider implements StorageUnitGroupCheckProvider {
    @Override
    public boolean applicable(String subType) {
        return ImmutableList.of(ResourceSubTypeEnum.GAUSSDB_DWS.getType(),
                ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType(), ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType(),
                ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType()).contains(subType);
    }

    @Override
    public boolean isSupportParallelStorage() {
        return Boolean.TRUE;
    }
}
