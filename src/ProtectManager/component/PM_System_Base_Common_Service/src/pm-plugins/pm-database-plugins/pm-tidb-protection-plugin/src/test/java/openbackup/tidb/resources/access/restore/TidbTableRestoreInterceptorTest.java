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
package openbackup.tidb.resources.access.restore;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.provider.TidbAgentProvider;
import openbackup.tidb.resources.access.service.TidbService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * TidbTableRestoreInterceptorTest
 *
 */
@RunWith(PowerMockRunner.class)
public class TidbTableRestoreInterceptorTest {
    @Mock
    private TidbService tidbService;

    @Mock
    private CopyRestApi copyRestApi;

    private TidbTableRestoreInterceptor tidbTableRestoreInterceptor;

    @Mock
    private ResourceService resourceService;

    @Mock
    private DefaultProtectAgentSelector defaultSelector;

    @Before
    public void setUp() {
        tidbTableRestoreInterceptor = new TidbTableRestoreInterceptor(tidbService, new TidbAgentProvider(tidbService),
            copyRestApi, resourceService, defaultSelector);
    }

    @Test
    public void applicable_test() {
        Assert.assertTrue(tidbTableRestoreInterceptor.applicable(ResourceSubTypeEnum.TIDB_TABLE.getType()));
    }
}
