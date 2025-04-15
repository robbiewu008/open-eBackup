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
package openbackup.oracle.provider;

import com.google.common.collect.ImmutableList;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.copy.CopyCommonInterceptor;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Optional;

/**
 * oracle 副本拦截器
 *
 */
@Component
@Slf4j
public class OracleCopyCommonInterceptor implements CopyCommonInterceptor {
    private static final List<String> ORACLE_SUB_TYPE_LIST = ImmutableList.of(ResourceSubTypeEnum.ORACLE.getType(),
            ResourceSubTypeEnum.ORACLE_CLUSTER.getType(), ResourceSubTypeEnum.ORACLE_CLUSTER_ENV.getType(),
            ResourceSubTypeEnum.ORACLE_CLUSTER_INSTANCE.getType());

    @Override
    public boolean applicable(String object) {
        return ORACLE_SUB_TYPE_LIST.contains(object);
    }

    @Override
    public void backupBuildCopyPostprocess(CopyInfo copyInfo) {
        JSONObject resourceProperties = JSONObject.fromObject(copyInfo.getResourceProperties());
        copyInfo.setIsStorageSnapshot(Optional.ofNullable(resourceProperties.getJSONObject("ext_parameters"))
            .orElseGet(JSONObject::new)
            .getBoolean("storage_snapshot_flag", false));
    }
}
