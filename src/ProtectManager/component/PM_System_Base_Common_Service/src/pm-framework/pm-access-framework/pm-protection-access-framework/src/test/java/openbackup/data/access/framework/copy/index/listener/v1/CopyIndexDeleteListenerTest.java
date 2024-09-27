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
package openbackup.data.access.framework.copy.index.listener.v1;

import openbackup.data.access.framework.copy.index.listener.v1.CopyIndexDeleteListener;
import openbackup.data.access.framework.core.common.constants.CopyIndexConstants;
import openbackup.system.base.common.utils.JSONObject;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.kafka.support.Acknowledgment;

/**
 * CopyIndexListenerTest LLT
 *
 * @author m00576658
 * @since 2021-03-22
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(CopyIndexDeleteListener.class)
@AutoConfigureMockMvc
public class CopyIndexDeleteListenerTest {
    @Mock
    private RedissonClient redissonClient;

    @InjectMocks
    private CopyIndexDeleteListener CopyDeleteListener;

    @Test
    public void testCopyIndexDeleteResponse() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE))).thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);

        JSONObject data = new JSONObject();
        data.set(CopyIndexConstants.STATUS, CopyIndexConstants.SUCCESS);
        CopyDeleteListener.copyIndexDeleteResponse(data.toString(), acknowledgment);
        Mockito.verify(acknowledgment, Mockito.times(2)).acknowledge();
    }
}