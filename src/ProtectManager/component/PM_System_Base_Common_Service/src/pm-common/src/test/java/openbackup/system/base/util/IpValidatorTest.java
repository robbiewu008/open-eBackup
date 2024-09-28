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
package openbackup.system.base.util;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseErrorInfo;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NodeControllerInfo;
import openbackup.system.base.util.IpValidator;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Objects;

/**
 * IP校验类测试
 *
 */
@RunWith(PowerMockRunner.class)
public class IpValidatorTest {
    @Mock
    private InfrastructureRestApi infrastructureRestApi;


    @InjectMocks
    private IpValidator ipValidator;

    private final String ipInA8000 = "172.16.14.15";

    /**
     * 初始化函数
     */
    @Before
    public void init() {
        NodeControllerInfo nodeControllerInfo = new NodeControllerInfo();
        nodeControllerInfo.setControl("0A");
        InfraResponseErrorInfo errorInfo = new InfraResponseErrorInfo();
        errorInfo.setErrId("123");
        errorInfo.setErrMsg("system error");

        InfraResponseWithError<NodeControllerInfo> mockRight = new InfraResponseWithError<>(nodeControllerInfo, null);
        InfraResponseWithError<NodeControllerInfo> mockError = new InfraResponseWithError<>(null, errorInfo);

        PowerMockito.when(infrastructureRestApi.getNodeInfoByPodIp(Mockito.argThat(
                ip -> !Objects.equals(ip, ipInA8000)))).thenReturn(mockError);
        PowerMockito.when(infrastructureRestApi.getNodeInfoByPodIp(Mockito.argThat(
                ip -> Objects.equals(ip, ipInA8000)))).thenReturn(mockRight);
    }

    /**
     * 用例场景：调用ipInA8000成功
     * 前置条件：1. 输入为A8000内部ip；2.输入为A8000外部ip
     * 检查点： 用例1返回True；用例2返回False
     */
    @Test
    public void call_isIpInA8000_success() {
        Assert.assertTrue(ipValidator.isIpInA8000(ipInA8000));
        Assert.assertFalse(ipValidator.isIpInA8000("192.168.1.1"));
    }

    /**
     * 用例场景：输入ip地址合法且不为A8000内部ip时，调用checkIp成功
     * 前置条件：输入ip地址合法且不为A8000内部ip
     * 检查点：不抛出异常。
     */
    @Test
    public void call_checkIp_success() {
        ipValidator.checkIp("192.168.142.15");
        ipValidator.checkIp("89.68.12.34");
    }

    /**
     * 用例场景：输入ip地址为A8000内部ip时，调用checkIp成功
     * 前置条件：输入ip地址为A8000内部ip
     * 检查点：抛出异常，且错误码为非法参数。
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_exception_if_ip_is_in_A8000_when_check() {
        ipValidator.checkIp(ipInA8000);
    }

    /**
     * 用例场景：输入ip地址不合法时，调用checkIp成功
     * 前置条件：输入ip地址不合法
     * 检查点：抛出异常，且错误码为非法参数。
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_exception_if_ip_is_illegal_when_check() {
        ipValidator.checkIp("256.1.1.1");
    }
}
