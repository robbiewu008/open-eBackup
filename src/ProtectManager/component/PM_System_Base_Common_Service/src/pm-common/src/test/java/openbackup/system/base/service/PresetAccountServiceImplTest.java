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
package openbackup.system.base.service;

import openbackup.system.base.common.utils.security.EncryptorUtil;
import openbackup.system.base.sdk.auth.model.dao.PresetAccountDao;
import openbackup.system.base.sdk.auth.model.dao.PresetAccountPo;
import openbackup.system.base.sdk.auth.model.request.PresetAccountRequest;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.service.PresetAccountServiceImpl;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述
 *
 * @author y30021475
 * @since 2023-08-07
 */
@RunWith(PowerMockRunner.class)
public class PresetAccountServiceImplTest {
    @Mock
    private PresetAccountDao presetAccountDao;

    @Mock
    private EncryptorUtil encryptorUtil;

    private PresetAccountServiceImpl presetAccountService;

    @Before
    public void init() {
        presetAccountService = new PresetAccountServiceImpl(presetAccountDao,encryptorUtil);
    }

    @Test
    public void setPresetAccountDaoTest() {
        PowerMockito.when(presetAccountDao.delete(Mockito.any())).thenReturn(1);
        PowerMockito.when(encryptorUtil.getEncryptPwd("Huawie@123")).thenReturn("AAAAAABBBBBCCCC");
        PowerMockito.when(presetAccountDao.insert(Mockito.any())).thenReturn(1);
        PresetAccountRequest presetAccountRequest = new PresetAccountRequest();
        presetAccountRequest.setUserName("name1");
        presetAccountRequest.setUserPwd("Huawie@123");
        presetAccountRequest.setSourceType("hcs");
        UuidObject uuidObject = presetAccountService.savePresetAccountAndPwd(presetAccountRequest);
        Assert.assertNotNull(uuidObject.getUuid());
    }

    @Test
    public void queryPresetAccountAndPwdByNameAndTypeTest() {
        PresetAccountPo presetAccountPo= new PresetAccountPo();
        presetAccountPo.setUserName("name1");
        presetAccountPo.setUserPwd("AAAAAABBBBBCCCC");
        presetAccountPo.setSourceType("hcs");
        PowerMockito.when(presetAccountDao.selectOne(Mockito.any())).thenReturn(presetAccountPo);
        PowerMockito.when(encryptorUtil.getDecryptPwd("AAAAAABBBBBCCCC")).thenReturn("Huawei@123");
        PresetAccountPo presetAccountPo1 = presetAccountService.queryPresetAccountAndPwdByNameAndType("name1", "hcs");
        Assert.assertEquals(presetAccountPo1.getUserPwd(), "Huawei@123");
    }
}
