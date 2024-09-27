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
package openbackup.data.access.framework.core.security.permission;

import openbackup.data.access.framework.core.security.permission.TrustAspect;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import org.aspectj.lang.ProceedingJoinPoint;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import javax.servlet.http.HttpServletRequest;

/**
 * TrustAspect llt
 *
 * @author g00500588
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/12/2
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {RequestContextHolder.class, ServletRequestAttributes.class})
public class TrustAspectTest {
    private ResourceService resourceService;

    private DeployTypeService deployTypeService;

    @InjectMocks
    private TrustAspect trustAspect;

    @Before
    public void init() {
        PowerMockito.mockStatic(RequestContextHolder.class);
        resourceService = PowerMockito.mock(ResourceService.class);

        trustAspect = new TrustAspect(resourceService, deployTypeService);
        Whitebox.setInternalState(trustAspect, "resourceService", resourceService);

        deployTypeService = PowerMockito.mock(DeployTypeService.class);
        Whitebox.setInternalState(trustAspect, "deployTypeService", deployTypeService);
    }

    /**
     * 用例场景：校验客户端ip是否授信
     * 前置条件：部署场景X8000
     * 检查点：从http header获取客户端ip成功
     */
    @Test
    public void check_client_ip_trust_success() throws Throwable {
        String clientIp = "192.168.162.190";
        String xForwardedValue = "192.168.162.190, 172.16.192.1";

        mockHttpHeaderXForwardedFor(xForwardedValue, DeployTypeEnum.X8000);
        StringBuilder clientIpFromHttpHeader = new StringBuilder();
        PowerMockito.when(resourceService.checkHostIfBeTrustedByEndpoint(clientIp, true)).thenAnswer(invocation -> {
            clientIpFromHttpHeader.append(invocation.getArgument(0).toString());
            System.out.println(invocation.getArgument(0).toString());
            return true;
        });
        ProceedingJoinPoint proceedingJoinPoint = PowerMockito.mock(ProceedingJoinPoint.class);

        trustAspect.checkClientIpTrust(proceedingJoinPoint, null);
        Assert.assertEquals(clientIp, clientIpFromHttpHeader.toString());
    }

    /**
     * 用例场景：校验客户端ip是否授信
     * 前置条件：部署场景防勒索
     * 检查点：防勒索只有内置agent不用校验客户端ip是否可信
     * */
    @Test
    public void check_client_ip_trust_success_when_hyper_detect_deploy_type() throws Throwable {
        String clientIp = "192.168.162.190";
        String xForwardedValue = "192.168.162.190, 172.16.192.1";

        mockHttpHeaderXForwardedFor(xForwardedValue, DeployTypeEnum.HYPER_DETECT);

        ProceedingJoinPoint proceedingJoinPoint = PowerMockito.mock(ProceedingJoinPoint.class);

        trustAspect.checkClientIpTrust(proceedingJoinPoint, null);

        Mockito.verify(resourceService, Mockito.times(0)).checkHostIfBeTrustedByEndpoint(clientIp, true);
    }

    private void mockHttpHeaderXForwardedFor(String xForwardedValue, DeployTypeEnum hyperDetect) {
        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(hyperDetect);

        HttpServletRequest request = PowerMockito.mock(HttpServletRequest.class);

        ServletRequestAttributes requestAttributes = PowerMockito.mock(ServletRequestAttributes.class);
        PowerMockito.when(RequestContextHolder.currentRequestAttributes()).thenReturn(requestAttributes);

        PowerMockito.when(requestAttributes.getRequest()).thenReturn(request);
        PowerMockito.when(request.getHeader("x-forwarded-for")).thenReturn(xForwardedValue);
    }
}
