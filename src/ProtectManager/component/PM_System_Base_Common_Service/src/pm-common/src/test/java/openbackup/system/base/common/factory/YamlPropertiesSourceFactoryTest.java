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
package openbackup.system.base.common.factory;

import openbackup.system.base.common.factory.YamlPropertiesSourceFactory;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.core.io.ClassPathResource;
import org.springframework.core.io.support.EncodedResource;

import java.io.IOException;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {YamlPropertiesSourceFactory.class})
public class YamlPropertiesSourceFactoryTest {
    @InjectMocks
    private YamlPropertiesSourceFactory yamlPropertiesSourceFactory;

    @Test
    public void createPropertySourceTest() throws IOException {
        ClassPathResource resource = new ClassPathResource("/conf/alarmI18nE/AlarmCommonEn.json");
        EncodedResource encodedResource = new EncodedResource(resource);
        yamlPropertiesSourceFactory.createPropertySource("AlarmCommonEn.json", encodedResource);
    }
}
