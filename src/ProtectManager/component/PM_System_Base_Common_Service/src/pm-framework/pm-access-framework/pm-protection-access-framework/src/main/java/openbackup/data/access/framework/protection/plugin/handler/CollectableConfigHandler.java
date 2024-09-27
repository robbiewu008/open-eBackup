/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.protection.plugin.handler;

import openbackup.data.protection.access.provider.sdk.plugin.CollectableConfig;
import openbackup.data.protection.access.provider.sdk.plugin.ResourceExtensionHandler;
import openbackup.system.base.common.utils.json.JsonUtil;

import com.fasterxml.jackson.core.type.TypeReference;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;

/**
 * The CollectableConfigHandler
 *
 * @author g30003063
 * @since 2022/6/7
 */
@Component
public class CollectableConfigHandler extends ResourceExtensionHandler<Object, List<CollectableConfig>> {
    @Override
    public String getNamePath() {
        return "functions.connection.dependency";
    }

    @Override
    public List<CollectableConfig> handle(final Object configObj, final Object params) {
        if (configObj == null) {
            return Collections.emptyList();
        }
        return JsonUtil.read(JsonUtil.json(configObj), new TypeReference<List<CollectableConfig>>() {
        });
    }
}