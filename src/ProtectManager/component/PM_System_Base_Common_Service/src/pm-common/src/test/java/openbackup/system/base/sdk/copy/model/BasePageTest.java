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
package openbackup.system.base.sdk.copy.model;

import openbackup.system.base.sdk.PageQueryRestApi;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.Environment;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.*;

/**
 * BasePage test
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = BasePage.class)
public class BasePageTest {

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private PageQueryRestApi pageQueryRestApi;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    private static final String RESOURCE_PROPERTIES
            = "{\"uuid\":\"fe22ee7e-8b96-4f86-a23d-d45265f77ef1\",\"name\":\"db151\",\"type\":\"Database\",\"path\":\"/tools/database\",\"sub_type\":\"Oracle\",\"created_time\":\"2020-12-29T23:20:43.109862\",\"ext_parameters\":{},\"parent_name\":\"OracleRedhat7_151\",\"parent_uuid\":\"483ed3f8-d66b-44d6-97e3-015ad26ef5bd\",\"root_uuid\":\"483ed3f8-d66b-44d6-97e3-015ad26ef5bd\",\"environment_name\":\"OracleRedhat7_151\",\"environment_uuid\":\"483ed3f8-d66b-44d6-97e3-015ad26ef5bd\",\"environment_endpoint\":\"51.6.136.155\",\"environment_os_type\":\"RedHat\",\"link_status\":\"0\",\"verify_status\":\"true\",\"db_role\":3,\"auth_type\":1,\"inst_name\":\"db151\",\"is_asminst\":0,\"version\":\"18.0.0.0.0\"}";

    @Test
    public void create_environment_success() {

        List<Environment> details = new ArrayList<>();
        Environment environment = new Environment();
        environment.setCluster(true);
        environment.setUserName("test_admin");
        environment.setPassword("test_admin");
        environment.setLinkStatus("1");
        environment.setEndpoint("125.11.22.33");

        BasePage.create(details);
    }

    private static BasePage<Copy> mokCopyData() {
        BasePage<Copy> copyBasePage = new BasePage<>();
        copyBasePage.setItems(getCopyList());
        copyBasePage.setPageNo(1);
        copyBasePage.setPages(2);
        copyBasePage.setTotal(2);
        return copyBasePage;
    }

    private static List<Copy> getCopyList() {
        List<Copy> copyList = new ArrayList<>();
        Copy copy = new Copy();
        copy.setGn(2);
        copy.setChainId("12");
        copy.setUuid(UUID.randomUUID().toString());

        Copy copy1 = new Copy();
        copy.setGn(3);
        copy.setChainId("34");
        copy.setUuid(UUID.randomUUID().toString());

        Copy copy2 = new Copy();
        copy.setGn(4);
        copy.setChainId("56");
        copy.setUuid(UUID.randomUUID().toString());
        copyList.add(copy);
        copyList.add(copy1);
        copyList.add(copy2);
        return copyList;
    }


}
