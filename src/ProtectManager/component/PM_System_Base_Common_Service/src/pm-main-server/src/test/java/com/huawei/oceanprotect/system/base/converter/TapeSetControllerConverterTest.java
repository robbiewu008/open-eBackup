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
package com.huawei.oceanprotect.system.base.converter;

import static org.mockito.ArgumentMatchers.any;
import static org.postgresql.hostchooser.HostRequirement.any;

import com.huawei.oceanprotect.repository.tapelibrary.service.bo.MediaSetBo;
import com.huawei.oceanprotect.repository.tapelibrary.service.repository.TapeStoragePoolRepository;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;

/**
 * 功能描述
 *
 * @since 2024-01-20
 */
@RunWith(PowerMockRunner.class)
public class TapeSetControllerConverterTest {

    @InjectMocks
    public TapeSetControllerConverter tapeSetControllerConverter;

    @Mock
    public TapeStoragePoolRepository tapeStoragePoolRepository;

    @Test
    public void test_get_name(){
        Assert.assertEquals("mediaSetId2name",tapeSetControllerConverter.getName());
    }

    @Test
    public void test_convert(){
        Mockito.when(tapeStoragePoolRepository.queryTapeStorageBo(any())).thenReturn(new MediaSetBo());
        Assert.assertEquals(1,tapeSetControllerConverter.convert(new ArrayList<String>(){{add("str");}}).size());
        Assert.assertEquals(1,tapeSetControllerConverter.convert(new ArrayList<String>(){{add(null);}}).size());

        Assert.assertEquals(1,tapeSetControllerConverter.convert(new ArrayList<Integer>(){{add(1);}}).size());

    }
}
