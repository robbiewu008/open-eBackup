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
package openbackup.oracle.controller;

import openbackup.oracle.controller.OracleCopyController;
import openbackup.oracle.service.OracleCopyService;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2024-01-16
 */
@RunWith(PowerMockRunner.class)
public class OracleCopyControllerTest {
    @InjectMocks
    private OracleCopyController oracleCopyController;

    @Mock
    private OracleCopyService oracleCopyService;

    @Test
    public void test_query_copy_by_scn(){
        oracleCopyController.queryCopyByScn("testResourcId","test");
    }
}

