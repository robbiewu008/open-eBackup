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
package openbackup.data.access.framework.protection.plugin;


import openbackup.data.access.framework.core.plugin.DefaultPluginConfigManager;
import openbackup.data.access.framework.protection.plugin.UnifiedResourceExtensionManager;
import openbackup.data.access.framework.protection.plugin.handler.CollectableConfigHandler;
import openbackup.data.protection.access.provider.sdk.plugin.CollectableConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginExtensionInvokeContext;
import openbackup.data.protection.access.provider.sdk.plugin.ResourceExtensionHandler;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.springframework.context.ApplicationContext;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 统一资源管理测试
 *
 */
public class UnifiedResourceExtensionManagerTest {
    private static final String AUTO_SCAN_CONFIG_PATH = "functions.scan.auto-scan";

    @Test
    public void result_can_return() {
        UnifiedResourceExtensionManager manager = mock_condition();
        PluginExtensionInvokeContext<Integer, String> connectionContext
            = new PluginExtensionInvokeContext<Integer, String>() {
            @Override
            public Integer getParams() {
                return 99;
            }
        };
        Assert.assertEquals(manager.invoke("Mysql", "functions.connection", connectionContext), "99");
        Assert.assertEquals(manager.invoke("Mysqlaaa", "functions.connection", connectionContext), "null");
        Assert.assertEquals(
            manager.invoke("Mysql", AUTO_SCAN_CONFIG_PATH, new PluginExtensionInvokeContext<Integer, List<String>>() {
                @Override
                public Integer getParams() {
                    return 99;
                }
            }).get(0), "99");
    }

    @Test
    public void retrieval_resource_environment_success() {
        UnifiedResourceExtensionManager manager = mock_condition();
        PluginExtensionInvokeContext<Object, List<CollectableConfig>> connectionContext
            = new PluginExtensionInvokeContext<Object, List<CollectableConfig>>() {
        };
        List<CollectableConfig> collectableConfigs = manager.invoke("Mysql", "functions.connection.dependency",
            connectionContext);
        Assert.assertEquals(collectableConfigs.size(), 2);
        Assert.assertEquals(collectableConfigs.get(0).getResource(), "node1");
        Assert.assertEquals(collectableConfigs.get(0).getEnvironment(), "env1");
        Assert.assertEquals(collectableConfigs.get(1).getResource(), "node2");
        Assert.assertEquals(collectableConfigs.get(1).getEnvironment(), "env2");
    }

    @Test(expected = LegoCheckedException.class)
    public void find_handler_exception_with_error_name_path() {
        UnifiedResourceExtensionManager manager = mock_condition();
        PluginExtensionInvokeContext<Integer, String> connectionContext
            = new PluginExtensionInvokeContext<Integer, String>() {
            @Override
            public Integer getParams() {
                return 99;
            }
        };
        manager.invoke("Mysql", "functions.connection.aa", connectionContext);
    }

    @Test(expected = LegoCheckedException.class)
    public void find_handler_exception_with_uncompatible_type() {
        UnifiedResourceExtensionManager manager = mock_condition();
        PluginExtensionInvokeContext<Integer, Boolean> connectionContext
            = new PluginExtensionInvokeContext<Integer, Boolean>() {
            @Override
            public Integer getParams() {
                return 99;
            }
        };
        manager.invoke("Mysql", "functions.connection", connectionContext);
    }

    private UnifiedResourceExtensionManager mock_condition() {
        DefaultPluginConfigManager pluginConfigManager = new DefaultPluginConfigManager();
        pluginConfigManager.init();
        // applicationContext
        ApplicationContext applicationContext = Mockito.mock(ApplicationContext.class);
        Map<String, ResourceExtensionHandler> map = new HashMap<>();
        map.put("connection", new ConnectionHandler());
        map.put("auto-scan", new AutoScanHandler());
        map.put("auto-scan2", new CollectableConfigHandler());
        Mockito.when(applicationContext.getBeansOfType(ResourceExtensionHandler.class)).thenReturn(map);

        UnifiedResourceExtensionManager manager = new UnifiedResourceExtensionManager(applicationContext,
            pluginConfigManager);
        manager.afterPropertiesSet();
        return manager;
    }

    private static class ConnectionHandler extends ResourceExtensionHandler<Integer, String> {
        @Override
        public String getNamePath() {
            return "functions.connection";
        }

        @Override
        public String handle(Object configObj, Integer params) {
            if (configObj == null) {
                return "null";
            }
            return "" + params;
        }
    }

    private static class AutoScanHandler extends ResourceExtensionHandler<Integer, List<String>> {
        @Override
        public String getNamePath() {
            return AUTO_SCAN_CONFIG_PATH;
        }

        @Override
        public List<String> handle(Object configObj, Integer params) {
            if (configObj == null) {
                return Collections.singletonList("null");
            }
            return Collections.singletonList("" + params);
        }
    }
}
