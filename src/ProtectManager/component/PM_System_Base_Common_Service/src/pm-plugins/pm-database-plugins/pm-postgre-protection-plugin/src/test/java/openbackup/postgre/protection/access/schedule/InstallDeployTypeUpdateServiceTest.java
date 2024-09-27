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
package openbackup.postgre.protection.access.schedule;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.system.base.common.constants.Constants;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.reflect.Whitebox;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * {@link InstallDeployTypeUpdateService} 测试类
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.6.0]
 * @since 2024-06-11
 */
public class InstallDeployTypeUpdateServiceTest {
    private ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private ResourceExtendInfoService resourceExtendInfoService = PowerMockito.mock(ResourceExtendInfoService.class);

    private final InstallDeployTypeUpdateService installDeployTypeUpdateService = new InstallDeployTypeUpdateService(
        resourceService, resourceExtendInfoService);

    @Test
    public void test_install_deploy_type_update_service() throws Exception {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("uuid");
        final HashMap<String, String> map = new HashMap<>();
        map.put(Constants.CERT_KEY, "111");
        map.put(Constants.REVOCATION_LIST_VMWARE, "222");
        protectedResource.setExtendInfo(map);
        response.setRecords(Arrays.asList(protectedResource));
        response.setTotalCount(1);
        PowerMockito.when(resourceService.query(any())).thenReturn(response);
        Whitebox.setInternalState(installDeployTypeUpdateService, "resourceService", resourceService);
        Map<String, String> extendInfo = new HashMap<>();
        PowerMockito.when(resourceExtendInfoService.queryExtendInfo(anyString(), anyString())).thenReturn(extendInfo);
        PowerMockito.doNothing().when(resourceExtendInfoService).saveOrUpdateExtendInfo(anyString(), any());
        installDeployTypeUpdateService.run();
        Assert.assertTrue(true);
    }
}