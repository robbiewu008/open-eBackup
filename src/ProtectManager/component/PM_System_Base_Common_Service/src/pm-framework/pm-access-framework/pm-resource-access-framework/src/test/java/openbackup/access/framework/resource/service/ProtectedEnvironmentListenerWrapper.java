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
package openbackup.access.framework.resource.service;

import openbackup.access.framework.resource.service.ProtectedEnvironmentListener;
import openbackup.access.framework.resource.service.ProtectedEnvironmentServiceImpl;
import openbackup.access.framework.resource.service.ProtectedResourceRepository;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.util.MessageTemplate;

import org.springframework.stereotype.Service;

/**
 * 描述
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-08
 */
@Service
public class ProtectedEnvironmentListenerWrapper extends ProtectedEnvironmentListener {
    /**
     * constructor
     *
     * @param protectedEnvironmentService protectedEnvironmentService
     * @param protectedResourceRepository protectedResourceRepository
     * @param messageTemplate messageTemplate
     * @param providerManager providerManager
     * @param resourceService resource service
     */
    public ProtectedEnvironmentListenerWrapper(ProtectedEnvironmentServiceImpl protectedEnvironmentService,
        ProtectedResourceRepository protectedResourceRepository, MessageTemplate<String> messageTemplate,
        ProviderManager providerManager, ResourceService resourceService) {
        super(protectedEnvironmentService, protectedResourceRepository, messageTemplate, providerManager,
            resourceService);
    }

    @Override
    public void afterPropertiesSet() throws Exception {
        return;
    }
}
