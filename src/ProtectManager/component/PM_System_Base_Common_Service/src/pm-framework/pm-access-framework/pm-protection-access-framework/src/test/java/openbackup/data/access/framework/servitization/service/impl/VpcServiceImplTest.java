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
package openbackup.data.access.framework.servitization.service.impl;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.servitization.dao.VpcEntityDao;
import openbackup.data.access.framework.servitization.service.impl.VpcServiceImpl;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * Vpc服务化测试用例
 *
 * @author l00853347
 * @since 2024-01-22
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(VpcServiceImpl.class)
public class VpcServiceImplTest {
    @InjectMocks
    private VpcServiceImpl vpcService;

    @Mock
    private VpcEntityDao vpcEntityDao;

    @Test
    public void test_save_vpc_info() {
        PowerMockito.when(vpcEntityDao.selectOne(any())).thenReturn(null);
        vpcService.saveVpcInfo("id", "id", "id");
    }

    @Test
    public void test_get_vpc_infos() {
        vpcService.getVpcInfos();
    }

    @Test
    public void test_delete_vpc_infos() {
        PowerMockito.when(vpcEntityDao.delete(any())).thenReturn(1);
        boolean deleteVpcInfo = vpcService.deleteVpcInfo("id");
        Assert.assertEquals(deleteVpcInfo, true);
    }

    @Test
    public void test_get_project_id_by_mark_id() {
        vpcService.getProjectIdByMarkId("id");
    }

    @Test
    public void test_vpc_info_entity_by_project_id() {
        vpcService.getVpcInfoEntityByProjectId("id");
    }

    @Test
    public void test_get_vpc_by_vpc_ids() {
        vpcService.getVpcByVpcIds(null);
    }
}
