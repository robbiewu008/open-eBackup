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
package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.sdk.cluster.enums.ClusterEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * ClusterEnum test
 *
 */
public class ClusterEnumTest {

    // TypeEnum
    @Test
    public void ClusterEnumd() {
        int LOCAL_TYPE = ClusterEnum.TypeEnum.LOCAL.getType();
        Assert.assertEquals(1, LOCAL_TYPE);

    }

    // StatusEnum
    @Test
    public void StatusEnum() {
        int NORMAL_STATUS = ClusterEnum.StatusEnum.NORMAL.getStatus();
        Assert.assertEquals(1, NORMAL_STATUS);

    }

    // StatusEnum
    @Test
    public void ModifyType() {
        int AUTH_TYPE = ClusterEnum.ModifyType.AUTH.getType();
        Assert.assertEquals(1, AUTH_TYPE);

    }

    // DoradoSelectTypeEnum
    @Test
    public void DoradoSelectTypeEnum() {
        String MANAGEMENT_TYPE = ClusterEnum.DoradoSelectTypeEnum.MANAGEMENT.getType();

        Assert.assertEquals("2", MANAGEMENT_TYPE);

    }


    // VerifyFlag
    @Test
    public void VerifyFlag() {
        String flag = ClusterEnum.VerifyFlag.AUTH_INVALID.getFlag();
        Assert.assertEquals("F", flag);

    }


    // OperateType
    @Test
    public void OperateType() {
        String operate = ClusterEnum.OperateType.DELETE_RELATION.getOperate();
        Assert.assertEquals("D", operate);

    }

    // EnableRelation
    @Test
    public void EnableRelation() {
        int ENABLE = ClusterEnum.EnableRelation.ENABLE.getRelation();
        Assert.assertEquals(1, ENABLE);

    }

    // RequestType
    @Test
    public void RequestType() {
        String EXTERNAL = ClusterEnum.RequestType.EXTERNAL.getType();
        Assert.assertEquals("external", EXTERNAL);
    }


    // DoradoCreateUserType
    @Test
    public void DoradoCreateUserType() {
        int USER_DEFINED_TYPE = ClusterEnum.DoradoCreateUserType.USER_DEFINED_TYPE.getUserType();
        Assert.assertEquals(1, USER_DEFINED_TYPE);
    }
}
