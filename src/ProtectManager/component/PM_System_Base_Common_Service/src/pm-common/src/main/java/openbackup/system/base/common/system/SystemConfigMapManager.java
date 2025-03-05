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
package openbackup.system.base.common.system;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.system.event.SystemConfigChangeEvent;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.infrastructure.InfrastructureService;
import openbackup.system.base.sdk.system.SystemConfigService;

import org.springframework.context.ApplicationContext;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;

/**
 * SystemConfigManager
 *
 */
@Component
@Slf4j
public class SystemConfigMapManager {
    private static final String PM_CONFIG_MAP_NAME = "protect-manager-conf";

    private final ConcurrentHashMap<String, String> cacheConfigmap;

    private final InfrastructureService infrastructureService;

    private final SystemConfigService systemConfigService;

    private final ApplicationContext applicationContext;

    SystemConfigMapManager(InfrastructureService infrastructureService, SystemConfigService systemConfigService,
        ApplicationContext applicationContext) {
        this.infrastructureService = infrastructureService;
        this.systemConfigService = systemConfigService;
        this.applicationContext = applicationContext;
        this.cacheConfigmap = new ConcurrentHashMap<String, String>() {{
            put(SystemConfigConstant.RUNNING_JOB_LIMIT_COUNT_ONE_NODE, "20");
            put(SystemConfigConstant.TOTAL_JOB_LIMIT_COUNT_ONE_NODE, "10000");
        }};
        getCache();
    }

    /**
     * 定时监控数据库修改
     */
    @Scheduled(cron = "0 0/1 * * * ?")
    public void monitDatabaseConfig() {
        Map<String, String> configs = refreshFromDataBase();
        if (!VerifyUtil.isEmpty(configs)) {
            SystemConfigChangeEvent systemConfigChangeEvent = new SystemConfigChangeEvent(this,
                Collections.unmodifiableMap(configs));
            applicationContext.publishEvent(systemConfigChangeEvent);
        }
    }

    private Map<String, String> refreshFromDataBase() {
        Map<String, String> configs = systemConfigService.getConfigValues(new ArrayList<>(cacheConfigmap.keySet()));
        for (Map.Entry<String, String> entry : configs.entrySet()) {
            if (Objects.equals(entry.getValue(), cacheConfigmap.get(entry.getKey()))) {
                configs.remove(entry.getKey());
            } else {
                cacheConfigmap.put(entry.getKey(), entry.getValue());
            }
        }
        return configs;
    }

    /**
     * 获取缓存值
     *
     * @param key key
     * @return value
     */
    public String getSystemConfig(String key) {
        return cacheConfigmap.get(key);
    }


    private void getCache() {
        Map<String, String> configMapValues = infrastructureService.getConfigMapValue(PM_CONFIG_MAP_NAME);
        cacheConfigmap.putAll(configMapValues);
        refreshFromDataBase();
    }
}
