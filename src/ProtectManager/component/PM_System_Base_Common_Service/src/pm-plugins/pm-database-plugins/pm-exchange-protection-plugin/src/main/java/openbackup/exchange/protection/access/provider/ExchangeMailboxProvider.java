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
package openbackup.exchange.protection.access.provider;

import openbackup.access.framework.resource.provider.DefaultResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import lombok.extern.slf4j.Slf4j;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.springframework.stereotype.Component;

/**
 * ExchangeMailboxProvider
 *
 */
@Slf4j
@Component
public class ExchangeMailboxProvider extends DefaultResourceProvider {
    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.EXCHANGE_MAILBOX.getType().equals(object.getSubType());
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        return true;
    }
}
