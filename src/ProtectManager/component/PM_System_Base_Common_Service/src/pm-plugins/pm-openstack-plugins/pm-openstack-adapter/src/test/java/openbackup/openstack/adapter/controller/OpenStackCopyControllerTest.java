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
package openbackup.openstack.adapter.controller;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.delete;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultHandlers.print;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import openbackup.openstack.adapter.controller.OpenStackCopyController;
import openbackup.openstack.adapter.service.OpenStackCopyAdapter;
import openbackup.system.base.common.utils.UUIDGenerator;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.web.servlet.MockMvc;

/**
 * {@link OpenStackCopyController} 测试类
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = OpenStackCopyController.class)
@AutoConfigureWebMvc
@AutoConfigureMockMvc
public class OpenStackCopyControllerTest {
    private static final String BASE_URL = "/v2/backup_copies";

    @Autowired
    private MockMvc mvc;

    @MockBean
    private OpenStackCopyAdapter adapter;

    /**
     * 用例名称：验证删除副本成功后，返回状态码200<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return200_when_deleteCopy() throws Exception {
        String copyId = UUIDGenerator.getUUID();
        mvc.perform(delete(BASE_URL + "/{id}", copyId)).andDo(print()).andExpect(status().isOk());
    }

    /**
     * 用例名称：验证查询所有副本成功后，返回状态码200<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return200_when_queryCopies() throws Exception {
        String resourceId = UUIDGenerator.getUUID();
        mvc.perform(get(BASE_URL).param("backup_job_id", resourceId)).andDo(print()).andExpect(status().isOk());
    }

    /**
     * 用例名称：验证查询所有副本时，backup_job_id参数不存在，返回状态码400<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return400_when_queryCopies_given_nullBackupJobId() throws Exception {
        mvc.perform(get(BASE_URL)).andDo(print()).andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：验证查询指定副本成功后，返回状态码200<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return200_when_queryCopy() throws Exception {
        String copyId = UUIDGenerator.getUUID();
        mvc.perform(get(BASE_URL + "/{id}", copyId)).andDo(print()).andExpect(status().isOk());
    }
}
