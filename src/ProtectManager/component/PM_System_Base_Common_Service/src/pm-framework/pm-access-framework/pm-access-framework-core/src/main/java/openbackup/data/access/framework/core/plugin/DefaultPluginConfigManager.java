/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package openbackup.data.access.framework.core.plugin;

import openbackup.data.protection.access.provider.sdk.plugin.PluginConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigConstants;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.security.exterattack.ExterAttack;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;

import org.springframework.core.io.Resource;
import org.springframework.core.io.support.PathMatchingResourcePatternResolver;
import org.springframework.core.io.support.ResourcePatternResolver;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;
import java.util.Optional;

/**
 * 插件管理实现
 *
 * @since 2022-05-23
 */
@Slf4j
public class DefaultPluginConfigManager implements PluginConfigManager {
    private List<PluginConfig> pluginConfigList;

    @ExterAttack
    @Override
    public void init() {
        pluginConfigList = new ArrayList<>();
        ResourcePatternResolver resolver = new PathMatchingResourcePatternResolver();
        Resource[] resources = null;
        try {
            resources = resolver.getResources("classpath*:plugins/*.json");
        } catch (IOException e) {
            log.error("loading plugin config file error");
        }
        if (resources != null) {
            for (Resource resource : resources) {
                try (InputStream inputStream = resource.getInputStream()) {
                    solveResource(inputStream).filter(this::checkValidPluginConfig).ifPresent(pluginConfigList::add);
                } catch (IOException ioException) {
                    log.error("loading plugin config file error, file name is {}", resource.getFilename());
                } catch (Exception e) {
                    log.error("solve plugin config file error, file name is {}", resource.getFilename());
                }
            }
        }
    }

    @Override
    public List<PluginConfig> getPluginConfigs() {
        return pluginConfigList;
    }

    @Override
    public Optional<PluginConfig> getPluginConfig(String subType) {
        return pluginConfigList.stream().filter(e -> Objects.equals(e.getSubType(), subType)).findFirst();
    }

    private Optional<PluginConfig> solveResource(InputStream inputStream) {
        JsonNode jsonNode = JsonUtil.read(inputStream);
        if (jsonNode == null) {
            return Optional.empty();
        }
        PluginConfig pluginConfig = new PluginConfig();
        pluginConfig.setConfigMap(new HashMap<>());
        Iterator<String> jsonNodeIterator = jsonNode.fieldNames();
        while (jsonNodeIterator.hasNext()) {
            String key = jsonNodeIterator.next();
            pluginConfig.getConfigMap().put(key, jsonNode.get(key));
        }

        pluginConfig.setType(
            Optional.ofNullable(jsonNode.get(PluginConfigConstants.TYPE)).map(JsonNode::textValue).orElse(null));
        pluginConfig.setSubType(
            Optional.ofNullable(jsonNode.get(PluginConfigConstants.SUB_TYPE)).map(JsonNode::textValue).orElse(null));
        return Optional.of(pluginConfig);
    }

    private boolean checkValidPluginConfig(PluginConfig pluginConfig) {
        if (pluginConfig == null) {
            return false;
        }
        if (Objects.isNull(pluginConfig.getType()) || Objects.isNull(pluginConfig.getSubType())) {
            log.error("plugin config is not valid.");
            return false;
        }
        return true;
    }
}
