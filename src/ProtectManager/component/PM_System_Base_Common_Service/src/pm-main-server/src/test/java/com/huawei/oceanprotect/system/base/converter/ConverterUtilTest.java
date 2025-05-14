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

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述
 *
 */

@RunWith(PowerMockRunner.class)
public class ConverterUtilTest {
    @Test
    public void test_mutina(){
        System.out.println(ConverterUtil.mutiNa(3));
        Assert.assertEquals("--,--,--",ConverterUtil.mutiNa(3));
    }

    @Test
    public void test_correct_ip_type(){
        Assert.assertTrue(ConverterUtil.correctIptype("ipv4"));
    }

}
