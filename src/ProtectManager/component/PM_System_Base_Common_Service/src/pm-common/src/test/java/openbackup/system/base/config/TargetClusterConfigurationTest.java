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
package openbackup.system.base.config;

import static org.mockito.ArgumentMatchers.any;

import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.sdk.cluster.TargetClusterRestApi;
import openbackup.system.base.util.RequestUriUtil;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.net.Proxy;

/**
 * The TargetClusterConfigurationTest
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {TargetClusterConfiguration.class, FeignBuilder.class, RequestUriUtil.class})
public class TargetClusterConfigurationTest {

    @InjectMocks
    private TargetClusterConfiguration targetClusterConfiguration;

    @Mock
    private TargetClusterRestApi targetClusterRestApi;

    @Mock
    private Proxy proxy;

    @Before
    public void init() {
        PowerMockito.mockStatic(RequestUriUtil.class);
        PowerMockito.when(RequestUriUtil.getDmeProxy(any())).thenReturn(proxy);

        PowerMockito.mockStatic(FeignBuilder.class);
        PowerMockito.when(FeignBuilder.buildDefaultTargetClusterClient(any(), any(), any()))
            .thenReturn(targetClusterRestApi);
    }

    @Test
    public void createTargetRequest() {
        targetClusterConfiguration.createTargetRequestBean(new DmaProxyProperties());
    }

    @Test
    public void createTargetApiWithDmeProxy() {
        targetClusterConfiguration.createTargetApiWithDmeProxy();
    }

    @Test
    public void createDefaultTargetApiWithDmeProxy() {
        targetClusterConfiguration.createDefaultTargetApiWithDmeProxy();
    }

    @Test
    public void targetClusterApiWithDmaProxyManagePort() {
        targetClusterConfiguration.createTargetRequestBeanManagePort(new DmaProxyProperties());
    }
}