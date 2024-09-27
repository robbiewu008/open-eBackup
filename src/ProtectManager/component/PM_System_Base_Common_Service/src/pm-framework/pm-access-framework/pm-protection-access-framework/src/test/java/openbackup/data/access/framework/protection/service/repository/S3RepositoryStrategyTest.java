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
package openbackup.data.access.framework.protection.service.repository;

import openbackup.data.access.framework.protection.service.repository.strategies.S3RepositoryStrategy;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import com.huawei.oceanprotect.repository.s3.entity.S3Storage;
import com.huawei.oceanprotect.repository.s3.service.S3StorageService;
import com.huawei.oceanprotect.system.base.cert.entity.ObjectCertInfo;
import com.huawei.oceanprotect.system.base.cert.service.ObjectCertDependencyService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * S3RepositoryStrategy测试类
 *
 * @author y30021475
 * @since 2023-09-06
 */
@RunWith(PowerMockRunner.class)
public class S3RepositoryStrategyTest {

    @InjectMocks
    private S3RepositoryStrategy s3RepositoryStrategy;

    @Mock
    private S3StorageService s3StorageService;

    @Mock
    private ObjectCertDependencyService objectCertDependencyService;

    @Test
    public void test_get_authentication() {
        S3Storage s3Storage = new S3Storage();
        s3Storage.setId("id1");
        s3Storage.setName("name1");
        s3Storage.setHttps(true);
        PowerMockito.when(s3StorageService.queryS3Storage(Mockito.anyString())).thenReturn(s3Storage);
        ObjectCertInfo objectCertInfo = new ObjectCertInfo();
        objectCertInfo.setCertId("cid");
        objectCertInfo.setCertName("certyy");
        PowerMockito.when(objectCertDependencyService.getCertNameByObjectId(Mockito.anyString())).thenReturn(objectCertInfo);
        Authentication authentication = s3RepositoryStrategy.getAuthentication("id1");
        Assert.assertEquals(authentication.getExtendInfo().get("certName"), "certyy");
    }
}
