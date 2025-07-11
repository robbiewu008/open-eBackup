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
package openbackup.access.framework.resource.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;

import org.springframework.stereotype.Component;

/**
 * 默认ResourceProvider
 *
 */
@Component("defaultResourceProvider")
@Slf4j
public class DefaultResourceProvider implements ResourceProvider {
    @Override
    public boolean applicable(ProtectedResource object) {
        return false;
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.debug("resource provider default. no need execute before create");
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.debug("resource provider default. no need execute before update");
    }
}
