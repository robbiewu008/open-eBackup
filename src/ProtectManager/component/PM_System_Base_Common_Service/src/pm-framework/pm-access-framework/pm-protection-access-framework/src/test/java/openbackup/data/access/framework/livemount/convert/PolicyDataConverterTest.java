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
package openbackup.data.access.framework.livemount.convert;

import openbackup.data.access.framework.livemount.converter.PolicyDataConverter;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Collection;

/**
 * test PolicyData Converter
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest
@ContextConfiguration(classes = {PolicyDataConverter.class})
public class PolicyDataConverterTest {
    @MockBean
    private LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    @Autowired
    private PolicyDataConverter policyDataConverter;

    /**
     * get name
     */
    @Test
    public void getName() {
        String name = policyDataConverter.getName();
        assert "live_mount_policy".equals(name);
    }

    /**
     * convert
     */
    @Test
    public void convert() {
        Collection<String> data = new ArrayList<>();
        data.add("1");
        data.add("2");
        Collection<?> convert = policyDataConverter.convert(data);
        Assert.assertEquals(convert.size(), 0);
    }
}
