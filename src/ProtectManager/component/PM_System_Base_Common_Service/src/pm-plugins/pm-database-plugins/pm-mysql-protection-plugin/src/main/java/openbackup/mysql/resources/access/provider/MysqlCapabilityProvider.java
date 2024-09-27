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
package openbackup.mysql.resources.access.provider;

import openbackup.data.protection.access.provider.sdk.copy.CapabilityProvider;
import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * Mysql副本支持操作
 *
 * @author w30042425
 * @since 2023-12-28
 */
@Component
@Slf4j
public class MysqlCapabilityProvider implements CapabilityProvider {
    private static final List<CopyFeatureEnum> SUPPORTED_FEATURES = Arrays.asList(CopyFeatureEnum.RESTORE,
        CopyFeatureEnum.MOUNT);

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType().equals(object);
    }

    @Override
    public List<CopyFeatureEnum> supportFeatures() {
        return SUPPORTED_FEATURES;
    }
}
