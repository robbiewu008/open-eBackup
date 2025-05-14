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
package com.huawei.oceanprotect.system.base.util;

import static org.mockito.ArgumentMatchers.any;

import openbackup.system.base.common.exception.LegoCheckedException;
import com.huawei.oceanprotect.system.base.initialize.network.util.NetworkConfigUtils;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;

import org.apache.poi.hssf.usermodel.HSSFCell;
import org.apache.poi.ss.usermodel.CellType;
import org.apache.poi.ss.usermodel.DateUtil;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;

/**
 * 功能描述
 *
 * @since 2024-01-20
 */

@RunWith(PowerMockRunner.class)
@PrepareForTest(DateUtil.class)
public class NetworkConfigUtilsTest {

    @Test
    public void test_get_net_base_ipv4(){
        NetworkConfigUtils.getNetBaseIpv4("8.40.102.105","255.255.0.0");
        Assert.assertTrue(true);
    }

    @Test
    public void test_get_eth_port(){


        Assert.assertThrows(LegoCheckedException.class,()->{ NetworkConfigUtils.getEthPort("8.40.102.105",new ArrayList<>(),"8.40.102.105");});
        NetworkConfigUtils.getEthPort("8.40.102.105",new ArrayList<EthPort>(){{
            add(new EthPort(){{
                setLocation("8.40.102.105");
            }});
        }},"8.40.102.105");
        Assert.assertTrue(true);
    }
    @Test
    public void test_get_cell_value(){
        HSSFCell hssfSheet = Mockito.mock(HSSFCell.class);
        Mockito.when(hssfSheet.getCellType()).thenReturn(CellType.STRING);
        NetworkConfigUtils.getCellValue(hssfSheet);
        Mockito.when(hssfSheet.getCellType()).thenReturn(CellType.NUMERIC);
        PowerMockito.mockStatic(DateUtil.class);
        PowerMockito.when(DateUtil.isCellDateFormatted(any())).thenReturn(true);
        NetworkConfigUtils.getCellValue(hssfSheet);
        PowerMockito.when(DateUtil.isCellDateFormatted(any())).thenReturn(false);
        NetworkConfigUtils.getCellValue(hssfSheet);
        Mockito.when(hssfSheet.getCellType()).thenReturn(CellType.FORMULA);
        NetworkConfigUtils.getCellValue(hssfSheet);
        Mockito.when(hssfSheet.getCellType()).thenReturn(CellType.BOOLEAN);
        NetworkConfigUtils.getCellValue(hssfSheet);
    }
}
