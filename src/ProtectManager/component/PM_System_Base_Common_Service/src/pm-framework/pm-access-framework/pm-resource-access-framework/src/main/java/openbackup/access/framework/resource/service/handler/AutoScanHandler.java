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
package openbackup.access.framework.resource.service.handler;

import openbackup.access.framework.resource.service.ProtectedEnvironmentListener;
import openbackup.data.protection.access.provider.sdk.plugin.ResourceExtensionHandler;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.util.MessageTemplate;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.BooleanUtils;
import org.springframework.stereotype.Component;

import java.util.Objects;
import java.util.Optional;

/**
 * 自动扫描handler
 *
 */
@Component
@Slf4j
public class AutoScanHandler extends ResourceExtensionHandler<ProtectedResource, Void> {
    private static final String AUTO_SCAN_CONFIG_PATH = "functions.scan.auto-scan";

    private final ResourceService resourceService;

    private final ProtectedEnvironmentService environmentService;

    private final SessionService sessionService;

    private MessageTemplate<String> messageTemplate;

    public AutoScanHandler(ResourceService resourceService, ProtectedEnvironmentService environmentService,
        SessionService sessionService, MessageTemplate<String> messageTemplate) {
        this.resourceService = resourceService;
        this.environmentService = environmentService;
        this.sessionService = sessionService;
        this.messageTemplate = messageTemplate;
    }

    @Override
    public String getNamePath() {
        return AUTO_SCAN_CONFIG_PATH;
    }

    @Override
    public Void handle(Object configObj, ProtectedResource params) {
        Boolean isAutoScan = Optional.ofNullable(configObj)
            .map(e -> BooleanUtils.toBoolean(e.toString()))
            .orElse(false);
        if (!isAutoScan || Objects.isNull(params)) {
            log.info("auto scan handler does not occur. resource id is: {}",
                Optional.ofNullable(params).map(ResourceBase::getUuid).orElse(null));
            return null;
        }
        log.info("auto scan handler occurs. resource id is: {}",
            Optional.ofNullable(params).map(ResourceBase::getUuid).orElse(null));
        String envId;
        if (params instanceof ProtectedEnvironment) {
            envId = params.getUuid();
        } else {
            Optional<ProtectedResource> rootResource = resourceService.getResourceById(params.getRootUuid());
            if (!rootResource.isPresent()) {
                throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                    "environment do not exist. id: " + params.getRootUuid());
            } else {
                envId = rootResource.get().getUuid();
            }
        }
        Optional<String> userId = Optional.ofNullable(sessionService.getCurrentUser()).map(TokenBo.UserBo::getId);
        if (!userId.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "do not find userId");
        }
        JSONObject messageData = new JSONObject();
        messageData.set("uuid", envId);
        messageTemplate.send(ProtectedEnvironmentListener.SCANNING_ENVIRONMENT_V2, messageData);
        return null;
    }
}
