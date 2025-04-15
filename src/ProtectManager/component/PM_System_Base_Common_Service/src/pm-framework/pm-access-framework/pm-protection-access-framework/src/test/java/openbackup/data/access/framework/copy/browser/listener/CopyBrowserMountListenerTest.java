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
package openbackup.data.access.framework.copy.browser.listener;

import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.CopyIndexConstants;
import openbackup.data.access.framework.core.common.enums.VmBrowserMountStatus;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.kafka.support.Acknowledgment;

import java.util.UUID;

/**
 * CopyBrowserMountListenerTest
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(CopyBrowserMountListener.class)
@AutoConfigureMockMvc
public class CopyBrowserMountListenerTest {
    @Mock
    private RedissonClient redissonClient;

    @Mock
    private CopyRestApi copyRestApi;

    @InjectMocks
    private CopyBrowserMountListener copyBrowserMountListener;

    @Before
    public void createMocks() {
        MockitoAnnotations.initMocks(this);
    }

    /*
     * 用例名称：副本细粒度浏览挂载成功
     * 前置条件：反馈的kafka消息中的status是fail，消息包含errorCode
     * check点：将副本状态标记为了“已挂载”，并反馈了对应的errorCode
     */
    @Test
    public void testCopyBrowseMountResponseSuccess() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
                redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);

        String copyId = UUID.randomUUID().toString();
        JSONObject data = new JSONObject();
        data.set(ContextConstants.COPY_ID, copyId);
        data.set(CopyIndexConstants.STATUS, "success");
        PowerMockito.doNothing()
            .when(copyRestApi)
            .updateCopyBrowseMountStatus(ArgumentMatchers.any(), ArgumentMatchers.any());
        copyBrowserMountListener.copyBrowseMountResponse(data.toString(), acknowledgment);
        Mockito.verify(copyRestApi)
            .updateCopyBrowseMountStatus(copyId, VmBrowserMountStatus.MOUNTED.getBrowserMountStatus());
    }

    /*
     * 用例名称：副本细粒度浏览挂载失败
     * 前置条件：反馈的kafka消息中的status是fail，消息包含errorCode
     * check点：将副本状态标记为了“挂载失败”，并反馈了对应的errorCode
     */
    @Test
    public void testCopyBrowseMountResponseFail() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
                redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);

        String copyId = UUID.randomUUID().toString();
        JSONObject data = new JSONObject();
        data.set(ContextConstants.COPY_ID, copyId);
        data.set(CopyIndexConstants.STATUS, "fail");
        PowerMockito.doNothing()
            .when(copyRestApi)
            .updateCopyBrowseMountStatus(ArgumentMatchers.any(), ArgumentMatchers.any());
        copyBrowserMountListener.copyBrowseMountResponse(data.toString(), acknowledgment);
        Mockito.verify(copyRestApi)
            .updateCopyBrowseMountStatus(copyId, VmBrowserMountStatus.MOUNT_FAIL.getBrowserMountStatus());
    }
}