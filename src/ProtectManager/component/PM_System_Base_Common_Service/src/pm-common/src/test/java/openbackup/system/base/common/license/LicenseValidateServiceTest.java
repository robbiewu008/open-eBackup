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
package openbackup.system.base.common.license;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.license.LicenseValidateService;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.license.LicenseServiceApi;
import openbackup.system.base.sdk.license.enums.FunctionEnum;
import openbackup.system.base.sdk.resource.ResourceRestApi;
import openbackup.system.base.sdk.resource.model.FileSetEntity;
import openbackup.system.base.service.DeployTypeService;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;
import java.util.UUID;

/**
 * License Validate Service Test
 *
 * @author c30044692
 * @since 2023/4/20
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = LicenseValidateService.class)
public class LicenseValidateServiceTest {

    @Autowired
    LicenseValidateService licenseValidateService;

    @MockBean
    private LicenseServiceApi licenseServiceApi;

    @MockBean
    private JobCenterRestApi jobCenterRestApi;

    @MockBean
    private ResourceRestApi resourceRestApi;
    @MockBean
    private DeployTypeService deployTypeService;

    @Before
    public void prepare() {
        FileSetEntity fileSetEntity = new FileSetEntity();
        fileSetEntity.setSubType("DBBackupAgent");
        Mockito.when(resourceRestApi.queryResource(ArgumentMatchers.anyString())).thenReturn(fileSetEntity);
    }
    /**
     * 测试场景：正确输入时测试是否可以正确处理 <br/>
     * 前置条件：入参正常 <br/>
     * 检查点：正确处理无异常抛出
     */
    @Test
    public void test_validate_success () {
        licenseValidateService.validate(FunctionEnum.BACKUP, UUID.randomUUID().toString(), UUID.randomUUID().toString());
    }

    /**
     * 测试场景：当functionLicense抛出异常时，是否能正确处理 <br/>
     * 前置条件：functionLicense异常 <br/>
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_exception_when_license_fail () {
        Mockito.doThrow(new LegoCheckedException("")).when(licenseServiceApi).functionLicense(ArgumentMatchers.anyString(), ArgumentMatchers.anyString());
        Assert.assertThrows(LegoCheckedException.class, () -> licenseValidateService.validate(FunctionEnum.BACKUP, UUID.randomUUID().toString(), UUID.randomUUID().toString()));
    }

    /**
     * 测试场景：jobId为null时，是否能正确处理 <br/>
     * 前置条件：jobId为null <br/>
     * 检查点：正常处理
     */
    @Test
    public void should_job_id_is_null_success () {
        licenseValidateService.validate("DBBackupAgent", FunctionEnum.BACKUP);
    }
}