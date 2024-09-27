/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * @author h30027154
 * @since 2022-06-23
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
