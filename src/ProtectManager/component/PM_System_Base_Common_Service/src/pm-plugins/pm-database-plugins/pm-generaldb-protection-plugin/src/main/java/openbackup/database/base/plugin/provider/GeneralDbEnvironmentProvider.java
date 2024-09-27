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
package openbackup.database.base.plugin.provider;

import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Objects;

/**
 * GeneralDb的EnvironmentProvider实现
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-26
 */
@Component
public class GeneralDbEnvironmentProvider implements EnvironmentProvider {
    private final UnifiedConnectionCheckProvider unifiedConnectionCheckProvider;

    public GeneralDbEnvironmentProvider(UnifiedConnectionCheckProvider unifiedConnectionCheckProvider) {
        this.unifiedConnectionCheckProvider = unifiedConnectionCheckProvider;
    }

    @Override
    public boolean applicable(String object) {
        return Objects.equals(object, ResourceSubTypeEnum.GENERAL_DB.getType());
    }

    @Override
    public void register(ProtectedEnvironment environment) {
    }

    @Override
    public void validate(ProtectedEnvironment environment) {
        unifiedConnectionCheckProvider.checkConnection(environment);
    }
}
