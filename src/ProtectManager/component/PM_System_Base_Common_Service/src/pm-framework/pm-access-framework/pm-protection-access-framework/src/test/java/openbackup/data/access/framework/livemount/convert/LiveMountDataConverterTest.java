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

import openbackup.data.access.framework.livemount.converter.LiveMountDataConverter;
import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;

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
 * test LiveMountData Converter
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest
@ContextConfiguration(classes = {LiveMountDataConverter.class})
public class LiveMountDataConverterTest {
    @MockBean
    private LiveMountEntityDao liveMountEntityDao;

    @Autowired
    private LiveMountDataConverter liveMountDataConverter;

    /**
     * get name
     */
    @Test
    public void getName() {
        String name = liveMountDataConverter.getName();
        assert "live_mount".equals(name);
    }

    /**
     * convert
     */
    @Test
    public void convert() {
        Collection<String> data = new ArrayList<>();
        data.add("3");
        data.add("4");
        Collection<?> convert = liveMountDataConverter.convert(data);
        Assert.assertEquals(convert.size(), 0);
    }
}
